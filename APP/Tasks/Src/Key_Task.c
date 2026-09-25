#include "Key_Task.h"

#include "IdleMeasure_Task.h"
#include "key.h"
#include "main.h"
#include "PageBack_Task.h"
#include "PageManager.h"
#include "Pages.h"


extern osThreadId_t IdleMonitorTaskHandle;

/**
 * @brief 按键回退
 */
void StartKeyTask(void *argument)
{
    /* USER CODE BEGIN StartKeyTask */
    uint8_t key_value=0;
    /* Infinite loop */
    for(;;)
    {
        key_value=KeyScan(0);
        switch (key_value)
        {
            case KEY1_VALUE:
                osThreadFlagsSet(IdleMonitorTaskHandle, IDLE_EXIT_FLAG);//空闲打断
                osThreadFlagsSet(PageBackTaskHandle,PAGE_BACK_FLAG);//发送回退页面的任务通知
                break;
        }
        osDelay(10);
    }
    /* USER CODE END StartKeyTask */
}
