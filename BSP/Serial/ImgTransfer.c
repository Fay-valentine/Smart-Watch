//
// 串口图片传输实现
// 帧格式: A5 5A | seq:2B(LE) | len:2B(LE) | payload:len B | crc16:2B(LE)
// crc16-CCITT(0xFFFF初值) 覆盖 seq..payload 共 (4+len) 字节
//

#include "ImgTransfer.h"
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"
#include "W25Q64.h"
#include "Serial_command.h"

extern osThreadId_t SerialTaskHandle;

#define RING_SIZE 512  //PC逐帧等ACK,同时在途<=1帧(136B),512是宽裕的安全余量

volatile bool g_img_downloading = false;

//ring:ISR只写head,任务只写tail,uint16自由递增,索引取 & (RING_SIZE-1)
static uint8_t  ring[RING_SIZE];
static volatile uint16_t ring_head = 0;
static volatile uint16_t ring_tail = 0;

//SerialTask栈只有512B,大缓冲一律static(单线程访问+有互斥锁,与Serial_Printf的buf同套路)
static uint8_t frame_buf[IMG_PAYLOAD_MAX + 8];
static uint8_t rd_buf[128];

static uint16_t exp_seq;          //期望的下一帧seq
static uint32_t wr_off;           //已写入长度
static uint32_t file_len;         //本次传输总长(imgopen声明)
static uint16_t expect_crc;       //整个文件的crc16(imgopen声明,close时与回读比对)
static uint32_t slot_base;
static uint32_t last_active_tick; //最近收到字节的时刻,用于超时中止

static uint16_t crc16_ccitt(const uint8_t* p, uint16_t n, uint16_t crc)
{
    for (uint16_t i = 0; i < n; i++)
    {
        crc ^= (uint16_t)p[i] << 8;
        for (uint8_t b = 0; b < 8; b++)
            crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
    return crc;
}

//手写解析"imgopen <slot> <len> <crc>":newlib sscanf的栈开销对任务栈是个负担
static int parse_u32(const char* s, const char** end)
{
    unsigned v = 0, n = 0;
    while (*s >= '0' && *s <= '9') { v = v * 10 + (*s - '0'); s++; n++; }
    if (end) *end = s;
    return n ? (int)v : -1;
}

static int parse_u32_hex(const char* s, const char** end)
{
    unsigned v = 0, n = 0;
    for (;;)
    {
        unsigned d;
        if (*s >= '0' && *s <= '9') d = *s - '0';
        else if (*s >= 'a' && *s <= 'f') d = *s - 'a' + 10;
        else if (*s >= 'A' && *s <= 'F') d = *s - 'A' + 10;
        else break;
        v = v * 16 + d; s++; n++;
    }
    if (end) *end = s;
    return n ? (int)v : -1;
}

void ImgTransfer_OpenCmd(const char* cmd)
{
    int slot, len, crc;
    const char* p = cmd + 8; //跳过"imgopen "
    while (*p == ' ') p++;
    if ((slot = parse_u32(p, &p)) < 0) { Serial_Printf("OPEN ERR SYNTAX\r\n"); return; }
    while (*p == ' ') p++;
    if ((len  = parse_u32(p, &p)) < 0) { Serial_Printf("OPEN ERR SYNTAX\r\n"); return; }
    while (*p == ' ') p++;
    if ((crc  = parse_u32_hex(p, &p)) < 0) { Serial_Printf("OPEN ERR SYNTAX\r\n"); return; }

    if (slot >= IMG_SLOT_MAX)               { Serial_Printf("OPEN ERR SLOT\r\n"); return; }
    if (len == 0 || len > IMG_SLOT_STRIDE)  { Serial_Printf("OPEN ERR LEN\r\n"); return; }

    slot_base  = IMG_SLOT0_BASE + slot * IMG_SLOT_STRIDE;
    file_len   = len;
    expect_crc = (uint16_t)crc;
    exp_seq    = 0;
    wr_off     = 0;
    ring_head = ring_tail = 0;   //丢弃下载模式切换前的残留字节
    last_active_tick = osKernelGetTickCount();
    g_img_downloading = true;    //此后RxEventCallback把数据分流到ring

    Serial_Printf("OPEN OK\r\n");
}

void ImgTransfer_OnRxISR(char* data, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++)
    {
        uint16_t next = (uint16_t)(ring_head + 1);
        if ((uint16_t)(next - ring_tail) > RING_SIZE) continue; //ring满:丢字节,
        ring[ring_head & (RING_SIZE - 1)] = (uint8_t)data[i];   //PC超时重发兜底
        ring_head = next;
    }
    last_active_tick = osKernelGetTickCount();
    osThreadFlagsSet(SerialTaskHandle, IMG_DATA_FLAG);
}

static void handle_frame(uint16_t seq, uint16_t len, const uint8_t* payload)
{
    if (seq == IMG_SEQ_CLOSE) //close帧:回读校验后落锤(注意:close不回ACK,回CLOSE OK/ERR)
    {
        g_img_downloading = false;
        if (wr_off != file_len)
        {
            Serial_Printf("CLOSE ERR LEN %d\r\n", wr_off);
            return;
        }
        uint16_t rd = 0xFFFF;
        uint32_t done = 0;
        while (done < file_len)
        {
            uint32_t n = (file_len - done > 128) ? 128 : (file_len - done);
            if (!W25Q64_Read_Data(slot_base + done, rd_buf, n))
            {
                Serial_Printf("CLOSE ERR READ\r\n");
                return;
            }
            rd = crc16_ccitt(rd_buf, (uint16_t)n, rd);
            done += n;
        }
        if (rd == expect_crc) Serial_Printf("CLOSE OK %d\r\n", file_len);
        else                  Serial_Printf("CLOSE ERR CRC %04X\r\n", rd);
        return;
    }

    if (seq == exp_seq)
    {
        if (wr_off + len > file_len) { Serial_Printf("ERR %d RANGE\r\n", seq); return; }

        uint32_t addr = slot_base + wr_off;
        if (wr_off % 4096 == 0)   //懒擦除:跨入新扇区先擦(含首扇区)
            if (!W25Q64_Sector_Erase(addr)) { Serial_Printf("ERR %d FLASH\r\n", seq); return; }

        if (len > 0 && !W25Q64_Page_Program(addr, (uint8_t*)payload, len))
        {
            Serial_Printf("ERR %d FLASH\r\n", seq);
            return;
        }
        exp_seq++;
        wr_off += len;
        Serial_Printf("ACK %d\r\n", seq);
    }
    else if ((uint16_t)(seq + 1) == exp_seq)  //重发帧:上次ACK丢在了路上,补ACK不重写
        Serial_Printf("ACK %d\r\n", seq);
    else
        Serial_Printf("ERR %d SEQ EXP %d\r\n", seq, exp_seq); //跳号:两端状态不一致,让PC中止重来
}

void ImgTransfer_Pump(void)
{
    if (!g_img_downloading) { ring_tail = ring_head; return; }

    for (;;)
    {
        uint16_t used = (uint16_t)(ring_head - ring_tail);
        if (used < 8) break;   //最小合法帧 = 头6 + crc2

        uint16_t r = ring_tail & (RING_SIZE - 1);
        if (ring[r] != 0xA5 || ring[(r + 1) & (RING_SIZE - 1)] != 0x5A)
        {
            ring_tail++;   //SOF失步:逐字节重同步(丢帧的字节会在这里被跳过)
            continue;
        }

        uint16_t len = ring[(r + 4) & (RING_SIZE - 1)] |        //len字段在字节4~5
                       ((uint16_t)ring[(r + 5) & (RING_SIZE - 1)] << 8);
        if (len > IMG_PAYLOAD_MAX) { ring_tail++; continue; } //长度非法,当噪声丢一步

        uint16_t total = 8 + len;   //SOF2+seq2+len2+payload+crc2
        if (used < total) break;    //帧未收齐,等下一次唤醒

        for (uint16_t i = 0; i < total; i++)
            frame_buf[i] = ring[(ring_tail + i) & (RING_SIZE - 1)];
        ring_tail = (uint16_t)(ring_tail + total);

        uint16_t seq  = frame_buf[2] | ((uint16_t)frame_buf[3] << 8);
        uint16_t fcrc = frame_buf[total - 2] | ((uint16_t)frame_buf[total - 1] << 8);
        uint16_t calc = crc16_ccitt(&frame_buf[2], (uint16_t)(4 + len), 0xFFFF); //覆盖seq..payload

        if (calc != fcrc) { Serial_Printf("ERR %d CRC\r\n", seq); continue; }
        handle_frame(seq, len, &frame_buf[6]);
    }
}

void ImgTransfer_Tick1s(void)
{
    if (!g_img_downloading) return;
    if (osKernelGetTickCount() - last_active_tick > 10000)
    {
        g_img_downloading = false;
        Serial_Printf("ERR TIMEOUT\r\n"); //防呆:半途断流不能一直吞着正常消息
    }
}
