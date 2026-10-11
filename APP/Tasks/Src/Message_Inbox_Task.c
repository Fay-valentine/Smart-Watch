//
// Created by a1191 on 2026/9/22.
//

#include "../Inc/Message_Inbox_Task.h"
#include "MessagePage_Inbox.h"
#include "cmsis_os2.h"
#include <stdbool.h>

#include "IdleMeasure_Task.h"
#include "lvgl.h"
#include "Serial_command.h"
#include "screens/ui_MessagePage.h"
#include "user_TasksInit.h"
#include "W25Q64.h"
#include "W25Q64_Task.h"

extern osThreadId_t IdleMonitorTaskHandle;
extern osMessageQueueId_t W25Q64QueueHandle;

void StartMsgInboxTask(void *argument)
{
    /* USER CODE BEGIN StartMsgInboxTask */
    uint8_t new_msg_count=0;
    // 等W25Q64启动恢复完成再开始搬运，避免与恢复回调并发写msg_list
    osThreadFlagsWait(W25Q64_MSG_INBOX_RESTORED_FLAG, osFlagsWaitAny, osWaitForever);
    /* Infinite loop */
    for(;;)
    {
        new_msg_count=MessagePage_Inbox_MsgQueue_To_MsgInbox(&message_page_inbox);
        if (new_msg_count!=0)
        {
            //收到消息，打断空闲
            osThreadFlagsSet(IdleMonitorTaskHandle, IDLE_EXIT_FLAG);
            //通知LvHandlerTask刷新消息窗口
            osThreadFlagsSet(LvHandlerTaskHandle,MSG_INBOX_CHANGED_FLAG);

            //把新消息放进W25Q64队列，让W25Q64Task保存消息
            StorageReq_t request;
            request.region=W25Q64_REGION_MSG_INBOX;

            //由于message_page_inbox.msg_list[i]，下标越小，消息越新,
            //所以从旧到新放进队列，使得W25Q64上越后的，为新消息
            for (int i = new_msg_count - 1; i >= 0; --i)
            {
                // 消息可能不足256字节，用实际长度，避免把垃圾数据写进Flash
                //strnlen 而不是 strlen：text[256] 如果恰好被填满没有 \0，strlen 会越界读；strnlen 限制在上限内。
                size_t text_len = strnlen(message_page_inbox.msg_list[i].text, MSG_MAX_SIZE);
                request.len = (uint16_t)text_len;

                memcpy(request.data, message_page_inbox.msg_list[i].text, text_len);

                if (osMessageQueuePut(W25Q64QueueHandle, &request, 0, 0) != osOK)
                {
                    Serial_Printf("[MsgInbox] queue full, msg dropped\r\n");
                }
            }
        }
        osDelay(20);
    }
    /* USER CODE END StartMsgInboxTask */
}
