//
// Created by a1191 on 2026/10/2.
//

#include "../Inc/DHT20_Task.h"

#include "cmsis_os2.h"
#include "HardWare_DataAccess.h"
#include "IdleMeasure_Task.h"
#include "PageManager.h"
#include "Pages.h"
#include "SerialTask.h"
#include "Serial_command.h"
#include "screens/ui_EnvironmentPage.h"

extern osThreadId_t IdleMonitorTaskHandle;

void StartAHT20Task(void *argument)
{
    /* USER CODE BEGIN StartAHT20Task */
    HWInterface.DHT20.Init();
    /* Infinite loop */
    for(;;)
    {
        if (Page_Get_NowPage()==&EnvironmentPage)//在环境页面才进行读取
        {
            HWInterface.DHT20.Read(&HWInterface.DHT20.temperature,&HWInterface.DHT20.humidity);
            Serial_Printf("temperature:%.1f°C,humidity:%.1f%%\r\n",HWInterface.DHT20.temperature,HWInterface.DHT20.humidity);
            osThreadFlagsSet(IdleMonitorTaskHandle,IDLE_EXIT_FLAG);
        }
        osDelay(2000);
    }
    /* USER CODE END StartAHT20Task */
}
