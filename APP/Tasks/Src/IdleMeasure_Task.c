//
// Created by a1191 on 2026/9/24.
//

#include "IdleMeasure_Task.h"

#include "cmsis_os2.h"
#include "HardWare_DataAccess.h"

#define UI_LIGHTOFF_VALUE 150    //单位：100ms

uint16_t IdleTimerCount_100ms=0;//一次计数即100ms
uint16_t ui_lightOff_value=UI_LIGHTOFF_VALUE;//默认15s降低亮度
uint16_t ui_sleep_value=UI_LIGHTOFF_VALUE+100;//降低亮度十秒后进入睡眠

extern osThreadId_t IdleMonitorTaskHandle;
extern osThreadId_t Sleep_TaskHandle;

static volatile bool idle_monitor_switch = true;//默认开启空闲监测

/**
 * @brief 开启/关闭空闲监测
 * @param switch_on true:开启，false:关闭
 */
void Idle_Monitor_Switch(const bool switch_on)
{
    idle_monitor_switch = switch_on;
}

void StartIdleMonitorTask(void *argument)
{
    /* USER CODE BEGIN StartIdleMonitorTask */
    uint32_t idle_flags=0;
    /* Infinite loop */
    for(;;)
    {
        idle_flags=osThreadFlagsWait(IDLE_ENTRY_FLAG | IDLE_EXIT_FLAG, osFlagsWaitAny, osWaitForever);

        if (idle_flags & IDLE_ENTRY_FLAG)
        {
            HWInterface.LCD.LightDown();
        }
        if (idle_flags & IDLE_EXIT_FLAG)
        {
            //恢复亮度
            HWInterface.LCD.Set_Light(HWInterface.LCD.Get_Brightness());
            //重置空闲时间计数
            IdleTimerCount_100ms=0;
        }
    }
    /* USER CODE END StartIdleMonitorTask */
}

/**
 * @brief 100ms调用一次，累加空闲时间计数，达到 降低亮度阈值 或 睡眠阈值，发送任务通知
 */
void IdleTimerCallback(void *argument)
{
    /* USER CODE BEGIN IdleTimerCallback */
    if (idle_monitor_switch==false)//不进行空闲监测
    {
        IdleTimerCount_100ms = 0;
        return;
    }

    IdleTimerCount_100ms++;
    if (IdleTimerCount_100ms==ui_lightOff_value)
    {
        osThreadFlagsSet(IdleMonitorTaskHandle,IDLE_ENTRY_FLAG);
    }
    if (IdleTimerCount_100ms==ui_sleep_value)
    {
        osThreadFlagsSet(Sleep_TaskHandle,SLEEP_ENTRY_FLAG);
    }
    /* USER CODE END IdleTimerCallback */
}

/**
 * @param lightOff_time_100ms 单位100ms
 */
void Idle_Monitor_Set_LightOff_Time(const uint16_t lightOff_time_100ms)
{
    ui_lightOff_value=lightOff_time_100ms;
}
