#include "Key_Task.h"

#include "key.h"
#include "main.h"

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
                //回退页面
                break;
        }

        osDelay(1);
    }
    /* USER CODE END StartKeyTask */
}
