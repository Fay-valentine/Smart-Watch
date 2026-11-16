//
// Created by a1191 on 2026/11/13.
//

#include "Serial_Command.h"

#include <string.h>
#include "HardWare_DataAccess.h"
#include "Serial.h"

char pending_cmd[64];              //只存最新一条指令
static volatile uint8_t cmd_pending_flag = 0;  //1 = 有指令待处理

typedef void(*CommandFunc_t)(char* cmd);

void cmd_reset_datetime_region(char* cmd);
void cmd_reset_msg_inbox_region(char* cmd);


typedef struct
{
    char* cmd;
    CommandFunc_t func;
}Command_t;

Command_t cmd_table[]=
{
    {"W25Q64:Reset datetime region",cmd_reset_datetime_region},
    {"W25Q64:Reset msg inbox region",cmd_reset_msg_inbox_region},
};

void cmd_reset_datetime_region(char* cmd)
{
    if (HWInterface.W25Q64.Log_Reset_Region(W25Q64_REGION_DATETIME))
    {
        Serial_Printf("Reset datetime region success\r\n");
    }
    else
    {
        Serial_Printf("Reset datetime region failed\r\n");
    }
}

void cmd_reset_msg_inbox_region(char* cmd)
{
    if (HWInterface.W25Q64.Log_Reset_Region(W25Q64_REGION_MSG_INBOX))
    {
        Serial_Printf("Reset msg inbox region success\r\n");
    }
    else
    {
        Serial_Printf("Reset msg inbox region failed\r\n");
    }
}

/**
 * @brief 查找命令函数表，并执行对应的命令函数
 * @param cmd 命令字符串
 */
void Serial_cmd_execute(char* cmd)
{
    if (cmd_pending_flag==0)
    {
        return;
    }

    cmd_pending_flag=0;
    for (int i=0;i<sizeof(cmd_table)/sizeof(cmd_table[0]);i++)
    {
        if (strcmp(cmd,cmd_table[i].cmd)==0)
        {
            cmd_table[i].func(cmd);
            return;
        }
    }
    Serial_Printf("Unknown command: %s\r\n",cmd);
}


void SerialCommand_CopyFromISR(char* data, uint16_t size)
{
    //对消息长度进行 截断保护（即限制数据的大小）
    uint16_t copy_size = size < sizeof(pending_cmd) - 1 ? size : sizeof(pending_cmd) - 1;

    //拷贝数据到pending_cmd
    memcpy(pending_cmd, data, copy_size);

    //加终止符‘\0’
    pending_cmd[copy_size] = '\0';

    //删除换行符
    char *nl = strpbrk(pending_cmd, "\r\n");
    if (nl) *nl = '\0';

    cmd_pending_flag = 1;
}