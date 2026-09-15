//
// Created by a1191 on 2026/9/13.
//

#include "../Inc/SerialTask.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <stdarg.h>
#include "HardWare_DataAccess.h"

char rxBuf[128];

void Serial_Send(uint8_t* data,uint16_t len)
{
    if (osMutexAcquire(SerialMutexHandle,osWaitForever)==osOK)//获取到串口资源
    {
        HAL_UART_Transmit_DMA(&huart1, data, len);//DMA串口发送

        osSemaphoreAcquire(SerialBinarySemHandle,osWaitForever);//等待串口发送完成

        osMutexRelease(SerialMutexHandle);//释放串口资源
    }
}

/**
 * @brief 格式化串口发送，不支持浮点
 */
void Serial_Printf(const char* fmt, ...)
{
    static char buf[128];    // static: 有互斥锁保护,不占任务栈
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len <= 0) return;
    if (len > (int)sizeof(buf) - 1) len = sizeof(buf) - 1;  // 截断保护

    Serial_Send((uint8_t*)buf, len);
}


void StartSerialTask(void *argument)
{
    /* USER CODE BEGIN StartSerialTask */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)rxBuf, sizeof(rxBuf));//开启接收
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);//关闭半传输中断

    HW_DateTimeTypeDef nowDateTime;
    /* Infinite loop */
    for(;;)
    {
        HWInterface.RealTimeClock.GetDateTime(&nowDateTime);
        //2026-9-15  15:22:55
        Serial_Printf("%d-%d-%d  %d:%d:%d week:%d",nowDateTime.Year,nowDateTime.Month,nowDateTime.Date,
                                               nowDateTime.Hours,nowDateTime.Minutes,nowDateTime.Seconds,nowDateTime.WeekDay);
        osDelay(1000);
    }
    /* USER CODE END StartSerialTask */
}

//回调
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart==&huart1)
    {
        osSemaphoreRelease(SerialBinarySemHandle);//释放二值信号量，表示发送完成
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart==&huart1)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1,(uint8_t*)rxBuf,sizeof(rxBuf));
        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx,DMA_IT_HT);
    }
}