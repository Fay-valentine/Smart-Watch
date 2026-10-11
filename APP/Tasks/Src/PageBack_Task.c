//
// Created by a1191 on 2026/9/16.
//

#include "../Inc/PageBack_Task.h"

#include "cmsis_os2.h"
#include "Key_Task.h"
#include "PageManager.h"
#include "Pages.h"
#include "user_TasksInit.h"
#include "screens/ui_HomePage.h"



#define BACK_COOLDOWN_MS  300  // 冷却时间：300ms 内只允许回退一次


void StartPageBackTask(void *argument)
{
    /* USER CODE BEGIN StartPageBackTask */
    /* Infinite loop */
    for(;;)
    {
        osThreadFlagsWait(PAGE_BACK_FLAG,osFlagsWaitAny,osWaitForever);//等待按键按下

        if (Page_Get_NowPage()->page_obj!=&ui_HomePage)//不在主页，回退页面
        {
            osThreadFlagsSet(LvHandlerTaskHandle,PAGE_BACK_FLAG);
        }

        ///防止快速连按
        osDelay(BACK_COOLDOWN_MS);
        osThreadFlagsClear(PAGE_BACK_FLAG); // 丢弃冷却期内按下的事件
    }
    /* USER CODE END StartPageBackTask */
}
