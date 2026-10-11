//
// Created by a1191 on 2026/9/22.
//

#include "../Inc/MessagePage_Inbox.h"
#include "lvgl.h"
#include "screens/ui_MessagePage.h"
#include <string.h>
#include "FreeRTOS.h"
#include "SerialTask.h"
#include "Serial_command.h"
#include "task.h"

MessagePage_Inbox_t message_page_inbox;

/**
 * @brief 初始化消息收件箱，将消息数量设为0，全部消息置为0，并创建传递 消息列表 的 消息队列
 */
void MessagePage_Inbox_Init(MessagePage_Inbox_t* messagePage_inbox)
{
    messagePage_inbox->msg_num=0;//消息数量为0
    memset(messagePage_inbox->msg_list,0,sizeof(messagePage_inbox->msg_list));//将所有消息置为0
    messagePage_inbox->queue_id=osMessageQueueNew(MSG_QUEUE_SIZE,sizeof(messagePage_inbox->msg_list[0]),NULL);

    if (messagePage_inbox->queue_id == NULL)
    {
        /* 堆耗尽*/
        Serial_Printf("[inbox] queue create failed, heap too small\r\n");
        configASSERT(0);      // FreeRTOS断言：Debug下挂停在此处，断点直接看到现场
    }
}

/**
 * @brief 将新消息插入到消息列表中
 */
static void MessagePage_Inbox_PushNewMsg(MessagePage_Inbox_t* messagePage_inbox, const Msg_t* msg)
{
    //将数组全部消息后移一位，移出数组尾部的消息
    for (int i = MSG_WINDOWS_NUM - 1; i > 0; --i)
    {
        messagePage_inbox->msg_list[i] = messagePage_inbox->msg_list[i - 1];
    }
    //将新消息插入到数组头部
    messagePage_inbox->msg_list[0] = *msg;

    //累加消息数量
    if (messagePage_inbox->msg_num < MSG_WINDOWS_NUM)
    {
        messagePage_inbox->msg_num++;
    }
}

/**
 * @brief 从 串口中断回调 拷贝 接收到的消息
 */
void MessagePage_Inbox_CopyMsgFromISR(const MessagePage_Inbox_t* messagePage_inbox, const char* data, const uint16_t size)
{
    //防止消息收件箱未被初始化 或 size == 0：ReceiveToIdle 有时会以 0 长度触发（比如对端只发了个打断字符），空消息没意义
    if (messagePage_inbox->queue_id == NULL || size == 0)
    {
        return;
    }

    //对消息长度进行 截断保护（即限制数据的大小）
    uint16_t copy_size=(size < MSG_MAX_SIZE - 1) ? size : (MSG_MAX_SIZE - 1);//-1是为了在末尾给终止符'\0'留空间

    //将数据拷贝到消息结构体中
    Msg_t msg={0};
    memcpy(msg.text,data,copy_size);

    //加终止符‘\0’
    msg.text[copy_size]='\0';

    //删除换行符
    char *nl = strpbrk(msg.text, "\r\n");
    if (nl) *nl = '\0';

    //将 经过处理的新消息 发送至消息队列
    osMessageQueuePut(messagePage_inbox->queue_id,&msg,0,0);//队列满则丢弃，不进行阻塞
}

/**
 * @brief 将 消息队列的消息 全部放进行 消息收件箱 中
 * @return true:有新消息，false:无新消息 或 消息队列未创建
 */
bool MessagePage_Inbox_MsgQueue_To_MsgInbox(MessagePage_Inbox_t* messagePage_inbox)
{
    if (messagePage_inbox->queue_id==NULL)
    {
        return false;
    }

    bool changed = false;
    Msg_t msg;
    while (osMessageQueueGet(messagePage_inbox->queue_id, &msg, NULL, 0) == osOK)  // 超时0=非阻塞
    {
        MessagePage_Inbox_PushNewMsg(messagePage_inbox, &msg);
        changed = true;
    }
    return changed;
}

/**
 * @brief 将消息收件箱中的消息 刷新到 消息窗口
 */
void MessagePage_render(const MessagePage_Inbox_t* messagePage_inbox)
{
    /* 存"指向全局变量的指针"而非控件地址本身：
   页面销毁重建后，透过它读到的永远是最新的控件指针（同 MenuEntry_t 的技巧） */
    lv_obj_t **labels[MSG_WINDOWS_NUM] = { &ui_Message1,&ui_Message2,&ui_Message3,&ui_Message4,&ui_Message5 };

    //将消息刷新到消息窗口
    for (int i = 0; i < MSG_WINDOWS_NUM; ++i)
    {
        //显示msg_num个窗口，并检测字符串不为空
        if (i<messagePage_inbox->msg_num && messagePage_inbox->msg_list[i].text[0]!='\0')
        {
            lv_label_set_text(*labels[i], messagePage_inbox->msg_list[i].text);     // 放消息
            //lv_obj_clear_flag(*btns[i], LV_OBJ_FLAG_HIDDEN);   // 显示窗
        }
        else
        {
            lv_label_set_text(*labels[i], "");                 // 删消息
            //lv_obj_add_flag(*btns[i], LV_OBJ_FLAG_HIDDEN);      // 隐藏窗
        }
    }
}