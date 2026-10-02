//
// Created by a1191 on 2026/10/2.
//

#include "../Inc/DHT20_Task.h"

#include "cmsis_os2.h"
#include "HardWare_DataAccess.h"
#include "SerialTask.h"

void StartAHT20Task(void *argument)
{
    /* USER CODE BEGIN StartAHT20Task */
    HWInterface.DHT20.Init();
    float temperature=0;
    float humidity=0;
    /* Infinite loop */
    for(;;)
    {
        HWInterface.DHT20.Read(&temperature,&humidity);
        Serial_Printf("temperature:%.1f°C,humidity:%.1f%%\r\n",temperature,humidity);
        osDelay(2000);
    }
    /* USER CODE END StartAHT20Task */
}
