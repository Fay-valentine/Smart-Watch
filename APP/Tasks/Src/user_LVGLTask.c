/* Private includes -----------------------------------------------------------*/
//includes

//bsp

// tasks
#include "user_TasksInit.h"
#include "user_LVGLTask.h"

//gui
#include "Key_Task.h"
#include "lvgl.h"
#include "PageManager.h"
#include "ui.h"

#include "StopwatchPage_Event.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/


/**
  * @brief  FreeRTOS Tick Hook, to increase the LVGL tick
  * @param  None
  * @retval None
  */
void TaskTickHook(void)
{
	//to increase the LVGL tick
	lv_tick_inc(1);

  Stopwatch_TimeRefresh();
}


/**
  * @brief  LVGL Handler task, to run the lvgl
  * @param  argument: Not used
  * @retval None
  */
void LvHandlerTask(void *argument)
{
  uint32_t _time = 1; // default delay time
  while(1)
  {
    _time = lv_timer_handler();

    //按键回退
    uint32_t page_flags = osThreadFlagsGet();// 非阻塞读
    if (page_flags & PAGE_BACK_FLAG)
    {
      osThreadFlagsClear(PAGE_BACK_FLAG);
      if (Page_Get_NowPage()->page_obj != &ui_HomePage)
      {
        Page_Back();
      }
    }

    // 限制最大休眠时间，保证按键队列被及时读取
    if(_time > 30)
    {
        _time = 30;
    }
    osDelay(_time);
  }
}
