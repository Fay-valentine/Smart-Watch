//
// Created by a1191 on 2026/9/13.
//

#include "../Inc/SerialTask.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <stdarg.h>
#include "Serial_command.h"
#include "HardWare_DataAccess.h"
#include "Bluetooth.h"
#include "MessagePage_Inbox.h"


void StartSerialTask(void *argument)
{
    /* USER CODE BEGIN StartSerialTask */
    HW_DateTimeTypeDef nowDateTime;

    HWInterface.Bluetooth.Set(true);//默认开启蓝牙
    uint8_t bluetooth_switch = 0;

    /* Infinite loop */
    for(;;)
    {
        //串口接收开关：有蓝牙开关事件立即唤醒，无事件最多等1秒（顺便作为发时间的节拍）
        //osThreadFlagsWait返回时会自动清除匹配到的flag
        osThreadFlagsWait(BT_STATE_CHANGED_FLAG, osFlagsWaitAny, 1000);

        uint8_t bluetooth_enable_flag = Bluetooth_GetState() ? 1U : 0U;//获取蓝牙开关状态

        if (bluetooth_enable_flag != bluetooth_switch)     /* 只在蓝牙状态变化时进行开关 */
        {
            if (bluetooth_enable_flag)//开启接收
            {
                HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)g_uart1_rxBuf, sizeof(g_uart1_rxBuf));
                __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
            }
            else
            {
                HAL_UART_AbortReceive(&huart1);   /*停止接收 */
            }
            bluetooth_switch = bluetooth_enable_flag;
        }

        //执行串口指令
        Serial_cmd_execute(pending_cmd);

    //     //循环发送时间
    //     HWInterface.RealTimeClock.GetDateTime(&nowDateTime);
    //     //2026-9-15  15:22:55
    //     Serial_Printf("%d-%d-%d  %d:%d:%d week:%d",nowDateTime.Year,nowDateTime.Month,nowDateTime.Date,
    //                                            nowDateTime.Hours,nowDateTime.Minutes,nowDateTime.Seconds,nowDateTime.WeekDay);
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
        if (Bluetooth_GetState()) //只有蓝牙开时，才重新开启接收（ISR里只读volatile bool，不调线程API）
        {
            //将接收到的消息拷贝到消息收件箱中
            MessagePage_Inbox_CopyMsgFromISR(&message_page_inbox,g_uart1_rxBuf,Size);
            SerialCommand_CopyFromISR(g_uart1_rxBuf,Size);
            //重新开启DMA接收，并关闭DMA接收过半中断
            HAL_UARTEx_ReceiveToIdle_DMA(&huart1, (uint8_t*)g_uart1_rxBuf, sizeof(g_uart1_rxBuf));
            __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
        }
    }
}