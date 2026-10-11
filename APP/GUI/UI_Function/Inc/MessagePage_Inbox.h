//
// Created by a1191 on 2026/9/22.
//

#ifndef INC_10_WATCH_MESSAGEPAGE_INBOX_H
#define INC_10_WATCH_MESSAGEPAGE_INBOX_H

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os2.h"

#define MSG_MAX_SIZE        256 //消息最大长度 256字节
#define MSG_WINDOWS_NUM     5   //5个消息窗口
#define MSG_QUEUE_SIZE      10  //消息队列大小 最多存10条消息

typedef struct
{
    char text[MSG_MAX_SIZE];
}Msg_t;//一条消息

typedef struct
{
    Msg_t msg_list[MSG_WINDOWS_NUM];//消息列表,下标越小，消息越新
    uint8_t msg_num;//当前消息数量
    osMessageQueueId_t queue_id;//消息队列ID
}MessagePage_Inbox_t;

extern MessagePage_Inbox_t message_page_inbox;

void MessagePage_Inbox_Init(MessagePage_Inbox_t* messagePage_inbox);
void MessagePage_Inbox_PushNewMsg(MessagePage_Inbox_t* messagePage_inbox, const Msg_t* msg);
void MessagePage_Inbox_CopyMsgFromISR(const MessagePage_Inbox_t* messagePage_inbox, const char* data, uint16_t size);
uint8_t MessagePage_Inbox_MsgQueue_To_MsgInbox(MessagePage_Inbox_t* messagePage_inbox);
void MessagePage_render(const MessagePage_Inbox_t* messagePage_inbox);

#endif //INC_10_WATCH_MESSAGEPAGE_INBOX_H