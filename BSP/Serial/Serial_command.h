//
// Created by a1191 on 2026/10/11.
//

#ifndef INC_10_WATCH_SERIAL_COMMAND_H
#define INC_10_WATCH_SERIAL_COMMAND_H

#define RX_BUF_SIZE 256
#include "cmsis_os2.h"

extern char g_uart1_rxBuf[RX_BUF_SIZE];
extern char pending_cmd[64];

extern osMutexId_t SerialMutexHandle;   //用于管理串口资源
extern osSemaphoreId_t SerialBinarySemHandle;//用于管理串口收发是否完成

void Serial_Printf(const char* fmt, ...);
void Serial_ResumeRx(void);

void SerialCommand_CopyFromISR(char* data, uint16_t size);
void Serial_cmd_execute(char* cmd);

#endif //INC_10_WATCH_SERIAL_COMMAND_H