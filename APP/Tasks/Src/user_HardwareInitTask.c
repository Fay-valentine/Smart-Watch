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
#include "ui.h"

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
    ui_init();

    xTaskResumeAll();
		vTaskDelete(NULL);
	}
}


