//
// Created by a1191 on 2026/11/14.
//

#include "Serial.h"

#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "Bluetooth.h"


#define SERIAL_BUF_SIZE        128U
#define SERIAL_MUTEX_WAIT_MS   200U
#define SERIAL_TX_WAIT_MS      100U


char g_uart1_rxBuf[RX_BUF_SIZE];

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
 * @brief 供睡眠唤醒后调用：若蓝牙处于开启状态，重新挂回空闲+DMA接收
 */
void Serial_ResumeRx(void)
{
    if (!Bluetooth_GetState()) return;
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)g_uart1_rxBuf, sizeof(g_uart1_rxBuf));
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
}

/**
 * @brief 格式化串口发送
 */
void Serial_Printf(const char* fmt, ...)
{
    static char buf[SERIAL_BUF_SIZE];   /* 所有任务共享，必须由 Mutex 保护 */
    va_list args;
    size_t len;

    //防止在调度器启动前调用本函数
    if (SerialMutexHandle == NULL)
    {
        return;
    }

    //获取串口互斥锁
    if (osMutexAcquire(SerialMutexHandle, SERIAL_MUTEX_WAIT_MS) != osOK)
    {
        return;   /* 拿不到锁：直接丢弃这条日志 */
    }


    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len <= 0)
    {
        osMutexRelease(SerialMutexHandle);
        return;
    }
    if (len > sizeof(buf) - 1)
    {
        len = sizeof(buf) - 1;  // 截断保护
    }

    //因为上一次如果超时/Abort，可能已经留下了一个迟到的信号量。先非阻塞地取一次，保证本次等待等到的是本次 DMA 的完成信号，而不是上一次的残留。
    osSemaphoreAcquire(SerialBinarySemHandle, 0);

    //进行DMA发送
    if (HAL_UART_Transmit_DMA(&huart1, (uint8_t*)buf, len) == HAL_OK)
    {
        //超时处理
        if (osSemaphoreAcquire(SerialBinarySemHandle,SERIAL_TX_WAIT_MS) != osOK)
        {
            //超时则中止发送，让下一次发送恢复正常状态
            HAL_UART_AbortTransmit(&huart1);
            //超时则清一遍 信号量，避免迟来的信号量污染下一次发送
            osSemaphoreAcquire(SerialBinarySemHandle, 0);
        }
    }
    osMutexRelease(SerialMutexHandle);
}


