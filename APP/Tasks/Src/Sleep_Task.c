//
// Created by a1191 on 2026/9/25.
//

#include "../Inc/Sleep_Task.h"

#include "cmsis_os2.h"
#include "CST816.h"
#include "HardWare_DataAccess.h"
#include "IdleMeasure_Task.h"
#include "lcd_init.h"
#include "FreeRTOS.h"
#include "key.h"
#include "task.h"
#include "usart.h"
#include "SerialTask.h"
#include "Bluetooth.h"
#include "main.h"
#include "Serial_command.h"
#include "stm32f4xx_it.h"

extern osThreadId_t LvHandlerTaskHandle;

void StartSleepTask(void *argument)
{
    /* USER CODE BEGIN StartSleepTask */
    /* Infinite loop */
    for(;;)
    {
        osThreadFlagsWait(SLEEP_ENTRY_FLAG, osFlagsWaitAny, osWaitForever);

        //sleep:
        IdleTimerCount_100ms = 0;//防止累加到超过熄屏阈值，失去熄屏功能
        /*-------------------------------关闭外设-------------------------------*/
        //sensors

        //usart
        HAL_UART_AbortReceive(&huart1);   //先关闭接收，否则会卡死
        HAL_UART_MspDeInit(&huart1);
        //LCD
        LCD_RES_Clr();//LCD 复位脚拉低，让屏幕驱动 IC 保持复位状态（不刷新、不发光）
        HWInterface.LCD.Light_Close();
        //touch
        CST816_Sleep();

        /*-------------------------------任务挂起-------------------------------*/
        //tasks
        vTaskSuspendAll();
        //Watch Dog


        do
        {
            //Systicks
            CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
            g_key1_flag = 0;
            //enter stop mode
            //所有代码停止，等待中断唤醒
            HAL_PWR_EnterSTOPMode(PWR_MAINREGULATOR_ON,PWR_STOPENTRY_WFI);

            /*-------------------------------MCU恢复-------------------------------*/
            //Systicks
            SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
            SystemClock_Config();                               // 重新配 PLL，恢复 100MHz 系统时钟

            /*-------------------------------判断唤醒 --------------------------------------*/
            if (!KEY1 || g_key1_flag == 1)
            {
                g_key1_flag = 0;
                break;                                          // 真唤醒，跳出继续恢复外设
            }

            IdleTimerCount_100ms = 0;                           // 假唤醒：外设还都处于关闭态，直接继续睡
        } while (1);

        /*--- 真唤醒：趁调度器还挂着，把外设全部恢复完，最后一步才放开调度器 ---------------- */
        /*     (若先 xTaskResumeAll 再恢复，高优先级 LvHandler 会对睡着的触摸芯片做 I2C     */
        /*      忙等，把本任务饿死在 LCD_Init 里 —— 即之前"按键唤不醒"的根因)               */

        //usart
        HAL_UART_MspInit(&huart1);
        Serial_ResumeRx();   /* 蓝牙开时重挂RX DMA接收(睡眠入口已AbortReceive) */
        //LCD
        HWInterface.LCD.Init();
        HWInterface.LCD.Light_Open();
        HWInterface.LCD.Set_Light(HWInterface.LCD.Get_Brightness());

        //touch
        CST816_Wakeup();

        //tasks
        xTaskResumeAll();

        //刷新一次首页时间
        osThreadFlagsSet(LvHandlerTaskHandle, UI_MAINPAGE_WAKE_REFRESH_FLAG);

        osDelay(100);
    }
    /* USER CODE END StartSleepTask */
}
