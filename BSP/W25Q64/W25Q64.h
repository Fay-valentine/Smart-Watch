//
// Created by a1191 on 2026/10/4.
//

#ifndef INC_10_WATCH_W25Q64_H
#define INC_10_WATCH_W25Q64_H
#include <stdint.h>

#define W25Q64_PAGE_PROGRAM_MAX_SIZE   256

//小数据用拷贝，用指针要管理内存，很麻烦，风险高
typedef struct {
    uint16_t region;    // 目标分区 ID：MSG / CONFIG / HEALTH...
    uint16_t len;       // 内容有效长度
    uint8_t  data[W25Q64_PAGE_PROGRAM_MAX_SIZE]; // 消息内容本体
} StorageReq_t;


void W25Q64_Init(void);
void W25Q64_ReadID(uint8_t* MID,uint16_t* DID);
uint8_t W25Q64_WriteEnable(void);
uint8_t W25Q64_Wait_Busy(void);
uint8_t W25Q64_Page_Program(uint32_t address, uint8_t* dataArray, uint16_t count);
uint8_t W25Q64_Sector_Erase(uint32_t address);
uint8_t W25Q64_Read_Data(uint32_t address, uint8_t* dataArray, uint32_t count);

#endif //INC_10_WATCH_W25Q64_H