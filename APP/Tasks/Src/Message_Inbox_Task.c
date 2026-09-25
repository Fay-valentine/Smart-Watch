//
// Created by a1191 on 2026/9/22.
//

#include "../Inc/Message_Inbox_Task.h"
#include "MessagePage_Inbox.h"
#include "cmsis_os2.h"
#include <stdbool.h>

#include "IdleMeasure_Task.h"
#include "lvgl.h"
#include "screens/ui_MessagePage.h"
#include "user_TasksInit.h"

extern osThreadId_t IdleMonitorTaskHandle;

void StartMsgInboxTask(void *argument)
{
    /* USER CODE BEGIN StartMsgInboxTask */
    bool changed=false;
    /* Infinite loop */
    for(;;)
    {
        changed=MessagePage_Inbox_MsgQueue_To_MsgInbox(&message_page_inbox);
        if (changed)
        {
            osThreadFlagsSet(IdleMonitorTaskHandle, IDLE_EXIT_FLAG); //收到消息，打断空闲
            osThreadFlagsSet(LvHandlerTaskHandle,MSG_INBOX_CHANGED_FLAG);//通知LvHandlerTask刷新消息窗口
        }
        osDelay(20);
    }
    /* USER CODE END StartMsgInboxTask */
}
