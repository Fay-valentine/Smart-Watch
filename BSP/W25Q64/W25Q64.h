//
// Created by a1191 on 2026/10/4.
//

#ifndef INC_10_WATCH_W25Q64_H
#define INC_10_WATCH_W25Q64_H
#include <stdint.h>

#include "cmsis_os2.h"

#define W25Q64_PAGE_PROGRAM_MAX_SIZE   256

//小数据用拷贝，用指针要管理内存，很麻烦，风险高
typedef struct {
    uint16_t region;    // 目标分区 ID：
    uint16_t len;       // 内容有效长度
    uint8_t  data[W25Q64_PAGE_PROGRAM_MAX_SIZE]; // 消息内容本体
} StorageReq_t;

#define W25Q64_DATETIME_MAGIC   0x5AA5      // 有效性标记：擦除后是0xFFFF，对不上就说明没存过
#define W25Q64_REGION_DATETIME  0x0001      // region ID，消息路由用
#define W25Q64_REGION_DATETIME_ADDR  0x001000  // 扇区1，避开扇区0的测试数据
typedef struct {
    uint16_t magic;      // 固定 0x5AA5，标记是否是有效存储
    uint8_t  year;       // 两位年，2000+year（和 HW_DateTimeTypeDef.Year 一致）
    uint8_t  month;      // 1~12
    uint8_t  day;        // 1~31
    uint8_t  hour;       // 0~23
    uint8_t  minute;     // 0~59
    uint8_t  second;     // 0~59
    uint8_t  checksum;   // 前8字节之和 & 0xFF
} DateTimeStore_t;       // 11字节：<256B，单页写完，不跨页




void W25Q64_Init(void);
void W25Q64_ReadID(uint8_t* MID,uint16_t* DID);
uint8_t W25Q64_WriteEnable(void);
uint8_t W25Q64_Wait_Busy(void);
uint8_t W25Q64_Page_Program(uint32_t address, uint8_t* dataArray, uint16_t count);
uint8_t W25Q64_Sector_Erase(uint32_t address);
uint8_t W25Q64_Read_Data(uint32_t address, uint8_t* dataArray, uint32_t count);
void W25Q64_Saved_Current_DateTime(void);
void W25Q64_RestoreDateTime(void);

#endif //INC_10_WATCH_W25Q64_H