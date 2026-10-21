import sys, time
import serial

def crc16_ccitt(data, crc=0xFFFF):
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF if crc & 0x8000 else (crc << 1) & 0xFFFF
    return crc

s = serial.Serial(sys.argv[1], 115200, timeout=0.5)

def drain(label, secs):
    end = time.time() + secs
    print(f"--- {label} ---")
    while time.time() < end:
        line = s.readline().decode(errors='ignore').strip()
        if line:
            print(repr(line))

drain("开机后3秒内手表主动输出(应该静默)", 3)

s.reset_input_buffer()
s.write(b"imgopen 0 128 %04X\r\n" % crc16_ccitt(bytes([0x11]) * 128))
drain("imgopen 之后", 3)

payload = bytes([0x11]) * 128
body = (0).to_bytes(2,'little') + (128).to_bytes(2,'little') + payload
frame = b'\xA5\x5A' + body + crc16_ccitt(body).to_bytes(2,'little')
s.write(frame)
drain("发送一个数据帧之后", 5)

close_body = (0xFFFF).to_bytes(2,'little') + (0).to_bytes(2,'little')
close = b'\xA5\x5A' + close_body + crc16_ccitt(close_body).to_bytes(2,'little')
s.write(close)
drain("发送close之后", 8)
s.close()
