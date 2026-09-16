#include "Key_Task.h"

#include "key.h"
#include "main.h"
#include "PageManager.h"
#include "Pages.h"




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
            case KEY1_VALUE://发送回退页面的任务通知
                osThreadFlagsSet(PageBackTaskHandle,PAGE_BACK_FLAG);
                break;
        }
        osDelay(10);
    }
    /* USER CODE END StartKeyTask */
}
