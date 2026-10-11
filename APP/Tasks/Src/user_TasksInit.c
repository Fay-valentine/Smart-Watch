/* Private includes -----------------------------------------------------------*/
//includes
#include "user_TasksInit.h"
//sys
#include "sys.h"
#include "stdio.h"

//bsp
#include "key.h"

//gui
#include "lvgl.h"

//tasks
#include "MessagePage_Inbox.h"
#include "user_HardwareInitTask.h"
#include "user_LVGLTask.h"
#include "W25Q64.h"

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
extern osTimerId_t IdleTimerHandle;
/* Timers --------------------------------------------------------------------*/


/* Tasks ---------------------------------------------------------------------*/
// Hardwares initialization
osThreadId_t HardwareInitTaskHandle;
const osThreadAttr_t HardwareInitTask_attributes = {
  .name = "HardwareInitTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityHigh,
};

// LVGL Handler task
osThreadId_t LvHandlerTaskHandle;
const osThreadAttr_t LvHandlerTask_attributes = {
  .name = "LvHandlerTask",
  .stack_size = 128 * 24,
  .priority = (osPriority_t) osPriorityLow1,
};


/* Message queues ------------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
void LvHandlerTask(void *argument);

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void User_Tasks_Init(void)
{
  /* add mutexes, ... */

  /* add semaphores, ... */

  /* start timers, add new ones, ... */

  /* add queues, ... */

  MessagePage_Inbox_Init(&message_page_inbox);//消息收件箱初始化
	/* add threads, ... */
  HardwareInitTaskHandle  = osThreadNew(HardwareInitTask, NULL, &HardwareInitTask_attributes);
  LvHandlerTaskHandle     = osThreadNew(LvHandlerTask, NULL, &LvHandlerTask_attributes);

  /* add events, ... */

	/* add  others ... */


}

