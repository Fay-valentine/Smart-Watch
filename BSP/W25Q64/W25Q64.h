//
// Created by a1191 on 2026/10/4.
//

#ifndef INC_10_WATCH_W25Q64_H
#define INC_10_WATCH_W25Q64_H
#include <stdint.h>

#include "cmsis_os2.h"



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