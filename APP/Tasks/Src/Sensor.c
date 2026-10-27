//
// Created by a1191 on 2026/10/2.
//

#include "../Inc/Sensor.h"

#include "cmsis_os2.h"
#include "HardWare_DataAccess.h"
#include "IdleMeasure_Task.h"
#include "PageManager.h"
#include "Pages.h"
#include "SerialTask.h"
#include "Serial_command.h"
#include "screens/ui_EnvironmentPage.h"

extern osThreadId_t IdleMonitorTaskHandle;

void StartSensorTask(void *argument)
{
    /* USER CODE BEGIN StartSensorTask */
    //DHT20
    HWInterface.DHT20.Init();
    float temperature=0.0f;
    float humidity=0.0f;

    //MPU6050
    uint8_t num=3;
    while (num && HWInterface.MPU6050.connection_error)
    {
        num--;
        HWInterface.MPU6050.connection_error=HWInterface.MPU6050.Init();
    }
    Serial_Printf("[MPU] init ret=%u (0=ok)\r\n", HWInterface.MPU6050.connection_error);

    HW_DateTimeTypeDef now_Date;
    HWInterface.RealTimeClock.GetDateTime(&now_Date);
    uint8_t day=now_Date.Date;
    uint8_t month=now_Date.Month;
    /* Infinite loop */
    for(;;)
    {
        //DHT20
        if (Page_Get_NowPage()==&EnvironmentPage)//在环境页面才进行读取
        {
            //不直接放进函数参数，是为了消除volatile float和float类型不匹配的警告
            HWInterface.DHT20.Read(&temperature,&humidity);
            HWInterface.DHT20.temperature = temperature;
            HWInterface.DHT20.humidity = humidity;
            Serial_Printf("temperature:%.1f°C,humidity:%.1f%%\r\n",HWInterface.DHT20.temperature,HWInterface.DHT20.humidity);
            osThreadFlagsSet(IdleMonitorTaskHandle,IDLE_EXIT_FLAG);
        }

        //MPU6050
        if (Page_Get_NowPage()==&StepsCountPage)
        {
            //跨天清零步数
            HWInterface.RealTimeClock.GetDateTime(&now_Date);
            if (now_Date.Date!=day || now_Date.Month!=month)
            {
                day=now_Date.Date;
                month=now_Date.Month;
                HWInterface.MPU6050.steps=0;
                //设置步数为0
                if (HWInterface.MPU6050.connection_error==0)
                {
                    HWInterface.MPU6050.SetSteps(0);
                }
            }
            //读取并更新步数
            if (HWInterface.MPU6050.connection_error==0)
            {
                HWInterface.MPU6050.steps=HWInterface.MPU6050.GetSteps();
            }
            Serial_Printf("steps:%d\r\n",HWInterface.MPU6050.steps);
            osThreadFlagsSet(IdleMonitorTaskHandle,IDLE_EXIT_FLAG);
        }
        osDelay(2000);
    }
    /* USER CODE END StartSensorTask */
}
