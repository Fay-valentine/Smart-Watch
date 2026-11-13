//
// Created by a1191 on 2026/10/8.
//

#include "../Inc/W25Q64_Task.h"

#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "HardwareInitTask.h"
#include "HardWare_DataAccess.h"
#include "MessagePage_Inbox.h"
#include "Serial_command.h"
#include "MessagePage_Inbox.h"

extern osMessageQueueId_t W25Q64QueueHandle;
extern osThreadId_t MsgInbox_TaskHandle;
extern osEventFlagsId_t BootEventHandle;

ReadMsgCtx_t read_msg_ctx={
    .got_msg_num = 0,
    .want_msg_num = MSG_WINDOWS_NUM,
};

StorageReq_t request;


void StartW25Q64Task(void *argument)
{
    /* USER CODE BEGIN StartW25Q64Task */
    //等待初始化完成
    osEventFlagsWait(BootEventHandle,
    BOOT_EVT_STORAGE_READY,
    osFlagsWaitAll|osFlagsNoClear,
    osWaitForever);


    /* Infinite loop */
    for(;;)
    {
        osMessageQueueGet(W25Q64QueueHandle,&request,NULL,osWaitForever);

        if (!W25Q64_Log_Append(request.region, request.data, request.len))
        {
            Serial_Printf("[W25Q64] append failed\r\n");
        }
    }
    /* USER CODE END StartW25Q64Task */
}

/**
 * @brief 读取N条消息到msg inbox中
 * @param payload 消息内容
 * @param payload_len 消息长度
 * @param arg 上下文指针
 * @return
 */
uint8_t W25Q64_Log_Foreach_Read_MsgInbox_cb(uint8_t* payload,uint16_t payload_len,void* arg)
{

    ReadMsgCtx_t *ctx = (ReadMsgCtx_t *)arg;

    //长度保护
    if (payload_len > MSG_MAX_SIZE - 1)
    {
        payload_len = MSG_MAX_SIZE - 1;
    }

    //将payload中的消息添加到msg inbox中
    Msg_t msg={0};
    memcpy(msg.text,payload,payload_len);
    msg.text[payload_len]='\0';

    //Foreach的遍历顺序是 新→旧
    message_page_inbox.msg_list[ctx->got_msg_num] = msg;
    ctx->got_msg_num++;
    message_page_inbox.msg_num = ctx->got_msg_num; //上电时收件箱为空，需要赋值

    //返回1表示继续遍历，返回0表示结束遍历
    if (ctx->got_msg_num>=ctx->want_msg_num)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
