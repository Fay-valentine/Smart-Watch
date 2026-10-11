//
// Created by a1191 on 2026/10/11.
//

#include "Serial_command.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "HardWare_DataAccess.h"
#include "Bluetooth.h"
#include "MessagePage_Inbox.h"
#include "SerialTask.h"


char g_uart1_rxBuf[RX_BUF_SIZE];

char pending_cmd[64];              //只存最新一条指令
static volatile uint8_t cmd_pending_flag = 0;  //1 = 有指令待处理

typedef void(*CommandFunc_t)(char* cmd);

void cmd_reset_datetime_region(char* cmd);

typedef struct
{
    char* cmd;
    CommandFunc_t func;
}Command_t;

Command_t cmd_table[]=
{
    {"W25Q64:Reset datetime region",cmd_reset_datetime_region},
};


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
    static char buf[128];    // static: 有互斥锁保护,不占任务栈
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len <= 0) return;
    if (len > (int)sizeof(buf) - 1) len = sizeof(buf) - 1;  // 截断保护

    Serial_Send((uint8_t*)buf, len);
}


void cmd_reset_datetime_region(char* cmd)
{
    if (HWInterface.W25Q64.Log_Reset_Region(W25Q64_REGION_DATETIME))
    {
        Serial_Printf("Reset datetime region success\r\n");
    }
    else
    {
        Serial_Printf("Reset datetime region failed\r\n");
    }
}

/**
 * @brief 查找命令函数表，并执行对应的命令函数
 * @param cmd 命令字符串
 */
void Serial_cmd_execute(char* cmd)
{
    if (cmd_pending_flag==0)
    {
        return;
    }

    cmd_pending_flag=0;
    for (int i=0;i<sizeof(cmd_table)/sizeof(cmd_table[0]);i++)
    {
        if (strcmp(cmd,cmd_table[i].cmd)==0)
        {
            cmd_table[i].func(cmd);
            return;
        }
    }
    Serial_Printf("Unknown command: %s\r\n",cmd);
}


void SerialCommand_CopyFromISR(char* data, uint16_t size)
{
    //对消息长度进行 截断保护（即限制数据的大小）
    uint16_t copy_size = size < sizeof(pending_cmd) - 1 ? size : sizeof(pending_cmd) - 1;

    //拷贝数据到pending_cmd
    memcpy(pending_cmd, data, copy_size);

    //加终止符‘\0’
    pending_cmd[copy_size] = '\0';

    //删除换行符
    char *nl = strpbrk(pending_cmd, "\r\n");
    if (nl) *nl = '\0';

    cmd_pending_flag = 1;
}