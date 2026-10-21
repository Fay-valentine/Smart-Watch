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
#include "tim.h"
#include "ImageFs.h"

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

	//PWM_LCD
	HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);

    // ui
    // LVGL and disp init
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();
    Page_Init();    /* 页面栈接管首页加载，替代 SquareLine 生成的 ui_init()，避免双重加载/重复建屏 */
    ImgFs_Init();   /* 注册'S'盘:只动LVGL全局链表,无RTOS等待,调度器暂停区可安全调用 */

    xTaskResumeAll();
    ImgFs_SelfTest(); /* 自检走Serial_Printf/W25Q64互斥锁,必须在调度器恢复之后 */
		vTaskDelete(NULL);
	}
}


