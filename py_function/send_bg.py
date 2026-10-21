# 通过串口把 LVGL 的 .bin 图片传输到手表 W25Q64 图片分区
# 用法: python send_bg.py <COM口> <bin文件> <槽号0/1/2>
# 例:   python send_bg.py COM7 bg_picture_bin/Evelyn_240X280.bin 0
# 前提: 手表已开机、蓝牙为开启状态(RX链路只在蓝牙开启时挂接)、屏幕别进入休眠

import sys, time, os
import serial

CHUNK = 128  # 与固件 IMG_PAYLOAD_MAX 一致

def crc16_ccitt(data, crc=0xFFFF):
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc

if len(sys.argv) != 4:
    sys.exit(__doc__)

port, path, slot = sys.argv[1], sys.argv[2], int(sys.argv[3])
data = open(path, 'rb').read()
fcrc = crc16_ccitt(data)
print(f"{os.path.basename(path)}: {len(data)}B, crc16={fcrc:04X}, slot={slot}")

s = serial.Serial(port, 115200, timeout=2)

def read_resp(want_prefix):
    """读一行。返回 (状态, 行内容): 'ok'匹配目标 / 'retry'对端要求重发当前帧 / 'dead'不可恢复 / 'again'超时"""
    line = s.readline().decode(errors='ignore').strip()
    if not line:
        return 'again', line
    if line.startswith(want_prefix):
        return 'ok', line
    if line.startswith('ERR') or line.startswith('CLOSE ERR') or line.startswith('OPEN ERR'):
        # CRC/FLASH类错误可重发当前帧; SEQ/TIMEOUT类直接中止, 由调用方用行内容判断
        if any(k in line for k in (' CRC', ' FLASH', ' RANGE')):
            return 'retry', line
        return 'dead', line
    return 'again', line  # 无关杂讯行, 继续读

def send_frame(seq, payload, want_prefix, deadline_s):
    body = seq.to_bytes(2, 'little') + len(payload).to_bytes(2, 'little') + payload
    frame = b'\xA5\x5A' + body + crc16_ccitt(body).to_bytes(2, 'little')
    deadline = time.time() + deadline_s
    while time.time() < deadline:
        s.write(frame)
        end = time.time() + 2.0            # 每个发帧动作2秒窗口, 没回话就重发
        while time.time() < end:
            st, line = read_resp(want_prefix)
            if st == 'ok':
                return True
            if st == 'dead':
                print(f"  abort @frame {seq}: {line}")
                return False
            if st == 'retry':
                print(f"  resp: {line} (重发)")
                break
    print(f"  frame {seq} 超时")
    return False

# 1) 握手: 文本命令走 pending_cmd, SerialTask 最多 1s 后才执行, 超时留 5s
s.reset_input_buffer()
s.write(f"imgopen {slot} {len(data)} {fcrc:04X}\r\n".encode())
deadline = time.time() + 5
opened = False
while time.time() < deadline and not opened:
    st, line = read_resp('OPEN OK')
    if st == 'ok':
        opened = True
    elif st == 'dead':
        sys.exit(f"imgopen failed: {line}")
if not opened:
    sys.exit("imgopen timeout")
print("OPEN OK, 开始传输...")

# 2) 数据帧
t0 = time.time()
for i in range(0, len(data), CHUNK):
    seq = i // CHUNK
    if not send_frame(seq, data[i:i+CHUNK], f'ACK {seq}', deadline_s=4):
        sys.exit(f"传输在第 {seq} 帧失败")
    if seq % 200 == 0:
        print(f"  {i*100//len(data):3d}%  frame {seq}  {time.time()-t0:.1f}s")

# 3) close 帧: 固件不回ACK, 回读整段Flash算CRC后才回 CLOSE OK/ERR
print("发送 close, 等待手表回读校验(约1~3s)...")
if not send_frame(0xFFFF, b'', 'CLOSE OK', deadline_s=15):
    sys.exit("close 失败")
print(f"RESULT: CLOSE OK, 全程 {time.time()-t0:.1f}s")
s.close()
