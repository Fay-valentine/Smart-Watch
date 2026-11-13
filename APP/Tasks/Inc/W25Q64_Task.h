//
// Created by a1191 on 2026/10/8.
//

#ifndef INC_10_WATCH_W25Q64_TASK_H
#define INC_10_WATCH_W25Q64_TASK_H

#include <stdint.h>

#include "MessagePage_Inbox.h"

uint8_t W25Q64_Log_Foreach_Read_MsgInbox_cb(uint8_t* payload,uint16_t payload_len,void* arg);

typedef struct {
    uint8_t want_msg_num;
    uint8_t got_msg_num;
} ReadMsgCtx_t;

extern ReadMsgCtx_t read_msg_ctx;


#endif //INC_10_WATCH_W25Q64_TASK_H