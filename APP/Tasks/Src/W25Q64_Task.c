//
// Created by a1191 on 2026/10/8.
//

#include "../Inc/W25Q64_Task.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "HardWare_DataAccess.h"
#include "usart.h"

void StartW25Q64Task(void *argument)
{
    /* USER CODE BEGIN StartW25Q64Task */
    uint8_t MID;							//定义用于存放MID号的变量
    uint16_t DID;							//定义用于存放DID号的变量

    uint8_t ArrayWrite[] = {0x01, 0x02, 0x03, 0x04};	//定义要写入数据的测试数组
    uint8_t ArrayRead[4];								//定义要读取数据的测试数组
    char msg[50];
    W25Q64_Init();


    W25Q64_ReadID(&MID, &DID);			//获取W25Q64的ID号
    sprintf(msg,"MID:%02X,DID:%04X\r\n",MID,DID);
    HAL_UART_Transmit(&huart1,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);

    /*W25Q64功能函数测试*/
    W25Q64_Sector_Erase(0x000000);					//扇区擦除
    W25Q64_Page_Program(0x000000, ArrayWrite, 4);	//将写入数据的测试数组写入到W25Q64中

    W25Q64_Read_Data(0x000000, ArrayRead, 4);		//读取刚写入的测试数据到读取数据的测试数组中

    sprintf(msg,"ArrayRead:%02X,%02X,%02X,%02X\r\n",ArrayRead[0],ArrayRead[1],ArrayRead[2],ArrayRead[3]);
    HAL_UART_Transmit(&huart1,(uint8_t*)msg,strlen(msg),HAL_MAX_DELAY);
    /* Infinite loop */
    for(;;)
    {
        osDelay(1);
    }
    /* USER CODE END StartW25Q64Task */
}
