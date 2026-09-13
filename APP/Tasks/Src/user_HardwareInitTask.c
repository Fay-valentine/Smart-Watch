/* Private includes -----------------------------------------------------------*/

// includes
// sys
#include "delay.h"
#include "sys.h"

// user
#include "user_TasksInit.h"

// bsp
#include "key.h"

// ui
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "PageManager.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  hardwares init task
  * @param  argument: Not used
  * @retval None
  */
void HardwareInitTask(void *argument)
{
	while(1)
	{
    vTaskSuspendAll();

	//delay
    delay_init();

	//key
	//Key_Port_Init();

    // ui
    // LVGL and disp init
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    Page_Init();    /* 页面栈接管首页加载，替代 SquareLine 生成的 ui_init()，避免双重加载/重复建屏 */

    xTaskResumeAll();
		vTaskDelete(NULL);
	}
}


