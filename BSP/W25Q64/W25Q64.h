//
// Created by a1191 on 2026/10/4.
//

#ifndef INC_10_WATCH_W25Q64_H
#define INC_10_WATCH_W25Q64_H
#include <stdint.h>

#include "cmsis_os2.h"

#define W25Q64_PAGE_PROGRAM_MAX_SIZE   256

#define W25Q64_DATETIME_REGION_ADDR  0x001000  // 扇区1，避开扇区0的测试数据

//SIZE 均以 字节 为单位
#define W25Q64_LOG_FRAME_HDR_SIZE    8      // magic2+seq2+region2+len1+ck1(Byte)
#define W25Q64_LOG_SLOT_MAGIC  0x5AA5 // 协议头magic，与擦除后的0xFFFF进行对比
#define W25Q64_LOG_SLOT_MAX_SIZE    256    // 单槽上限 = 一页
#define W25Q64_LOG_SECTOR_SIZE 4096


enum W25Q64_Region_List{
    W25Q64_REGION_DATETIME  = 0x0001, // 日期时间：状态型数据，读"最后一条"
    W25Q64_REGION_MSG_INBOX = 0x0002, // 收件箱：流型数据，Foreach遍历/Reset清空
};

//小数据用拷贝，用指针要管理内存，很麻烦，风险高
typedef struct {
    uint16_t region;    // 目标分区 ID：
    uint16_t len;       // 内容有效长度
    uint8_t  data[W25Q64_PAGE_PROGRAM_MAX_SIZE]; // 消息内容本体
} StorageReq_t;



/* 头结构体：只在 RAM 里用，不代表 Flash 布局 */
typedef struct {
    uint16_t magic;
    uint16_t seq;
    uint16_t region;
    uint8_t  len;
    uint8_t  checksum;
} W25Q64_Log_Frame_Hdr_t;

typedef struct// 纯数据，帧头帧尾都归日志层管
{
    uint8_t year,month, day;// 两位年，2000+year
    uint8_t hour, minute, second;
} DateTimeData_t;

//用于控制读取方式，比如读取多少条数据，读取特定数据等;返回0则停止调用
typedef uint8_t(*W25Q64_Log_Foreach_Read_cb)(uint8_t* payload,uint16_t payload_len,void* arg);

void W25Q64_Init(void);
void W25Q64_ReadID(uint8_t* MID,uint16_t* DID);
uint8_t W25Q64_WriteEnable(void);
uint8_t W25Q64_Wait_Busy(void);
uint8_t W25Q64_Page_Program(uint32_t address, uint8_t* dataArray, uint16_t count);
uint8_t W25Q64_Sector_Erase(uint32_t address);
uint8_t W25Q64_Read_Data(uint32_t address, uint8_t* dataArray, uint32_t count);

uint8_t W25Q64_Log_Init_AllRegion(void);
uint8_t W25Q64_Log_Append(uint16_t region,const uint8_t* payload,uint16_t payload_len);
uint8_t W25Q64_Log_Read_LastSlot(uint16_t region,uint8_t* payload,uint16_t payload_max_len,uint16_t* read_len);
uint8_t W25Q64_Log_Foreach_Read(uint16_t region,W25Q64_Log_Foreach_Read_cb cb,void* arg);
uint8_t W25Q64_Log_Reset_Region(uint16_t region);


void W25Q64_Saved_Current_DateTime(void);
void W25Q64_RestoreDateTime(void);

#endif //INC_10_WATCH_W25Q64_H