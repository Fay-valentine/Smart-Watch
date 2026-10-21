/* Private includes -----------------------------------------------------------*/
//includes

//bsp

// tasks
#include "user_TasksInit.h"
#include "user_LVGLTask.h"

//gui
#include "HomePage_RTC.h"
#include "HomePage_Events.h"
#include "IdleMeasure_Task.h"
#include "Key_Task.h"
#include "lvgl.h"
#include "MessagePage_Inbox.h"
#include "Message_Inbox_Task.h"
#include "PageBack_Task.h"
#include "PageManager.h"
#include "Pages.h"
#include "Sleep_Task.h"
#include "ui.h"

#include "StopwatchPage_Event.h"
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern osThreadId_t IdleMonitorTaskHandle;
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

    //空闲打断
    if (lv_disp_get_inactive_time(NULL)<1000)//1s内有触摸屏幕进行操作，打断空闲
    {
      osThreadFlagsSet(IdleMonitorTaskHandle,IDLE_EXIT_FLAG);
    }

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

    //消息页面刷新
    uint32_t msg_flags = osThreadFlagsGet();
    if (msg_flags & MSG_INBOX_CHANGED_FLAG)
    {
      osThreadFlagsClear(MSG_INBOX_CHANGED_FLAG);
      if (Page_Get_NowPage() == &MessagePage)
      {
        MessagePage_render(&message_page_inbox);
      }
    }

    //唤醒强制重刷
    uint32_t wake_flags = osThreadFlagsGet();
    if (wake_flags & UI_MAINPAGE_WAKE_REFRESH_FLAG)
    {
      osThreadFlagsClear(UI_MAINPAGE_WAKE_REFRESH_FLAG);
      HomePage_RTC_ForceFresh();          // 让下一次lv_timer（首页时间显示部分）必产生脏区
      // 让整个当前页面变成脏区，强制重绘；如果不放入脏区，复位后GRAM（GRAM还在，屏幕就还会显示旧的画面）被删了（现在这个硬件不会）
      //屏幕上就显示黑屏，因为lvgl不知道这块区域要进行刷新，不会要求重发像素，写到GRAM里
      lv_obj_invalidate(lv_scr_act());
    }

    //背景图切换/开机补投:LVGL非线程安全,set_src只能在GUI任务这里做
    //每拍无条件调用:无pending时是零成本return;开机第一次tick在此补投Init埋下的默认槽
    uint32_t bg_flags = osThreadFlagsGet();
    if (bg_flags & BG_SWITCH_FLAG)
    {
      osThreadFlagsClear(BG_SWITCH_FLAG); //请求只是提醒循环"有新的要处理",实际处理在下面
    }
    HomePage_ApplyPendingBg();

    // 限制最大休眠时间，保证按键队列被及时读取
    if(_time > 30)
    {
        _time = 30;
    }
    osDelay(_time);
  }
}
