# 串口图片传输使用说明

> 功能:把 PC 上的 LVGL `.bin` 图片文件通过串口写入手表 W25Q64 Flash 的图片分区,固件回读校验完整性。
>
> 相关代码:固件 `BSP/Serial/ImgTransfer.c/.h`,PC 端 `send_bg.py`(正式)、`probe.py`(调试)。
> 图片分区:`W25Q64.h` 宏 `IMG_SLOT0_BASE / IMG_SLOT_STRIDE`(见 `ImgTransfer.h`)。

---

## 1. 系统全貌

```
PC                                    手表
┌─────────────┐   115200 UART   ┌──────────────────────────┐
│ send_bg.py  │ ───────────────> │ RxEventCallback         │
│  切帧+算CRC  │   A5 5A帧+文本   │   └─下载模式→ring buffer │
│             │ <─────────────── │ SerialTask              │
│  等ACK/CLOSE│   ACK/CLOSE OK   │   └─Pump: 解析帧         │
└─────────────┘                  │       └─擦除/页编程→W25Q64│
                                 │       └─close时回读CRC判定│
                                 └──────────────────────────┘
```

三个环节各管一段,互不越界:

| 环节 | 职责 |
|---|---|
| PC(`send_bg.py`) | 算整文件 CRC → 切 128B 帧 → 逐帧等 ACK,失败重发 |
| 固件(`ImgTransfer`) | 擦除+页编程写入 Flash → close 时**回读整段重算 CRC** 做终局判定 |
| 显示(lv_fs 驱动,待写) | 按槽号把像素逐行喂给 LVGL 渲染 |

**只要看到 `CLOSE OK`,就可以无条件相信 Flash 里那份数据与硬盘源文件逐字节一致**(校验是回读 Flash 本身做的,写入中途出错、坏 cell 都逃不掉)。

---

## 2. 使用前提

1. USB-TTL 接 huart1,接线可靠、共地;
2. 手表**开机且清醒**(屏幕亮,按过按键)——STOP 模式下串口收发没有电力,必须先唤醒再传输;
3. **蓝牙开关为开**(RX DMA 链路只在蓝牙开启时挂接,这是工程原有架构);
4. PC 装好 pyserial:`pip install pyserial`;
5. 确认端口号:`python -m serial.tools.list_ports`(换 USB 口后端口可能变)。

> 固件已保证:**传输期间不会自动入睡**(Sleep_Task 会推迟入睡请求直到传输结束)。
> 但若传输开始前已经睡着,串口是完全静默的——传输不会把你叫醒。

## 3. 标准用法

```bash
python send_bg.py <COM口> <bin文件路径> <槽号 0|1|2>

# 示例:把 Velina 放进 2 号槽
python send_bg.py COM14 bg_picture_bin/Velina_240X280.bin 2
```

正常输出:

```
Velina_240X280.bin: 134404B, crc16=212B, slot=2   ← 文件概览+算好的整文件CRC
OPEN OK, 开始传输...                               ← 手表已进入下载模式
   0%  frame 0  0.1s
  95%  frame 1000  18.5s                           ← 每200帧一报进度
发送 close, 等待手表回读校验(约1~3s)...
RESULT: CLOSE OK, 全程 21.3s                       ← 终局判定
```

一张 132KB 图约 21 秒。**换图 = 对同一槽重跑命令**,协议第一步自带擦除,重发即覆盖,没有任何手动擦除操作。

### 图片槽位表

| 槽号 | Flash 地址 | 当前内容 |
|---|---|---|
| 0 | `0x101000` | Evelyn_240X280.bin |
| 1 | `0x141000` | Leimi_240X280.bin |
| 2 | `0x181000` | Velina_240X280.bin |

槽步长 `0x40000`(256KB)> 单图 134,404B;基址 4KB 扇区对齐。以后 lv_fs 显示层按这张表找图。

### 新图制作流程

1. 用 SquareLine / [LVGL Image Converter](https://lvgl.io/tools/imageconverter) 导出:`TrueColor`、`RGB565`、输出格式 `.bin`,尺寸 **240×280**;
2. 核对文件大小必须是 `240 × 280 × 2 + 4 = 134404 B`(开头 4 字节是 LVGL 文件头,传输时**原样保留**,显示层要用);
3. `send_bg.py` 传入目标槽;
4. `CLOSE OK` 即完成。

## 4. 协议参考(自己写其它发送端时用)

### 4.1 阶段一:文本握手(走普通串口命令路径)

```
PC → imgopen <slot> <len> <crc16hex>\r\n     例: imgopen 0 134404 212B
手表 → OPEN OK   或   OPEN ERR SYNTAX|SLOT|LEN
```

### 4.2 阶段二:二进制帧(下载模式下,数据不进收件箱)

帧格式(总长 = 8 + len):

```
偏移  0  1  2   3   4   5   6 ... 6+len-1   crc_lo crc_hi
    ┌──┬──┬───┬───┬───┬───┬──────────────┬─────────────┐
    │A5│5A│seq lo│seq hi│len lo│len hi│ payload(len B) │ CRC16 LE  │
    └──┴──┴───┴───┴───┴───┴──────────────┴─────────────┘
```

- `seq`:从 0 递增,小端;`len`:payload 长度,≤128,小端;
- `CRC16-CCITT`(多项式 0x1021,初值 0xFFFF)覆盖 **seq 起至 payload 末**(即偏移 2~6+len);
- 手表回 `ACK <seq>\r\n`;出错回 `ERR <seq> <CRC|SEQ|FLASH|RANGE>\r\n`;
- **payload 固定 128B 且槽基址 4KB 对齐** → 任意帧的写入偏移都是 128 的倍数,永不跨 256B 页,固件不做拆页;
- 擦除为懒擦除:写到 `offset % 4096 == 0` 时才擦该扇区;
- **幂等**:`seq == 已收-1` 的重发帧只补 ACK、不重写 Flash。所以"超时重发同 seq"永远安全。

### 4.3 阶段三:close

```
PC → 特殊帧 seq=0xFFFF, len=0
手表 → 回读整段Flash重算CRC → CLOSE OK <len>  或  CLOSE ERR CRC <算出的值> / CLOSE ERR LEN <实收>
```

### 4.4 保护机制

- **10s 静默超时**:下载模式下 10 秒没收到任何字节 → 固件自动退出下载模式并回 `ERR TIMEOUT`,之后蓝牙消息恢复正常;
- **ring buffer 512B**:逐帧 ACK 保证在途帧 ≤1 个,写满丢新字节由 PC 超时重发兜底。

## 5. 故障对号入座

| 现象 | 原因 | 处理 |
|---|---|---|
| `imgopen timeout` | 手表睡了 / 蓝牙关了 / 接线松 | 唤醒屏幕、开蓝牙、重跑命令 |
| `OPEN ERR SLOT` | 槽号 ≥ 3 | 改参数 |
| `OPEN ERR LEN` | 文件超 256KB 或为 0 | 检查图片尺寸/导出设置 |
| `... SEQ ...` 中止 | 两端帧号不同步(一般断过重连) | 整条命令重跑 |
| `ERR TIMEOUT` | 传输中途手表侧 10s 没收到数据 | 查接线,重跑 |
| `CLOSE ERR CRC` | 数据没写对 | 重跑;连续两次失败查线缆/共地 |
| 传输中途 Ctrl+C / 拔线 | 槽里是半张图 | 无害,10s 后自动退出下载模式,重跑覆盖 |

**铁律:任何失败,原命令重跑一遍即可,不需要清理、不需要复位。**

## 6. 调试工具 `probe.py`

`python probe.py COM14`。行为与 `send_bg.py` 相同,但每步之间**把串口收到的全部字节原样打印**。当怀疑的对象是"固件行为本身"(而不是"传输链路出错")时用它——本功能开发期就是靠它的分支 trace 抓出帧偏移 bug 的。固件侧若要加临时打印,照抄 `Serial_Printf` + static 缓冲的惯例即可,但注意第 7 节的栈问题。

## 7. 已知注意事项与坑

1. **CubeMX 重新生成会把 SerialTask 栈打回 `128*4`(512B)**,而 `sscanf` 之类大栈函数会在任务切换时引爆 HardFault(症状滞后、现场与真凶无关)。`freertos.c` 中 `SerialTask_attributes.stack_size` 应保持 **`512 * 4`**。每次用 CubeMX 改工程后必查这一行。
2. **`imgopen` 文本会被收件箱存成一条消息**(调试日志 `[W25Q64] append ok: region=2`)。需要过滤时在 `MessagePage_Inbox_CopyMsgFromISR` 里跳过以 `imgopen` 开头的行。
3. `ImgTransfer.c` 的大缓冲(`ring`/`frame_buf`/`rd_buf`)都是 static:任务栈只有 2KB,不要改回栈上分配。
4. 传输期间不要操作手表(按屏、切页),避免与 W25Q64 互斥锁竞争拖慢 ACK。
5. 图片区从 `0x101000` 起,与日志区(`0x001000`/`0x002000`)互不重叠;新增分区时别往 `0x101000 ~ 0x1C0FFF` 里放。

## 8. 文件清单与职责

| 文件 | 职责 |
|---|---|
| `BSP/Serial/ImgTransfer.h` | 分区宏、帧常量、接口 |
| `BSP/Serial/ImgTransfer.c` | 状态机:`OpenCmd`/`OnRxISR`(塞ring+唤醒任务)/`Pump`(解析+写Flash+ACK)/`Tick1s`(超时中止) |
| `BSP/Serial/Serial_command.c` | `imgopen` 前缀分流进 `Serial_cmd_execute` |
| `APP/Tasks/Src/SerialTask.c` | ISR 按 `g_img_downloading` 分流;收到帧立即 `Pump` |
| `APP/Tasks/Src/Sleep_Task.c` | 下载模式期间推迟入睡 |
| `Core/Src/freertos.c` | SerialTask 栈 2KB |
| `send_bg.py` | PC 发送端 |
| `probe.py` | 逐帧全量回显的调试发送端 |
