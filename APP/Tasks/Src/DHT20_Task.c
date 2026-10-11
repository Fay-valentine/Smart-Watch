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
    float temperature=0.0f;
    float humidity=0.0f;
    /* Infinite loop */
    for(;;)
    {
        if (Page_Get_NowPage()==&EnvironmentPage)//在环境页面才进行读取
        {
            //不直接放进函数参数，是为了消除volatile float和float类型不匹配的警告
            HWInterface.DHT20.Read(&temperature,&humidity);
            HWInterface.DHT20.temperature = temperature;
            HWInterface.DHT20.humidity = humidity;
            Serial_Printf("temperature:%.1f°C,humidity:%.1f%%\r\n",HWInterface.DHT20.temperature,HWInterface.DHT20.humidity);
            osThreadFlagsSet(IdleMonitorTaskHandle,IDLE_EXIT_FLAG);
        }
        osDelay(2000);
    }
    /* USER CODE END StartAHT20Task */
}
