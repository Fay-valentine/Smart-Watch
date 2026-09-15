//
// Created by a1191 on 2026/9/13.
//

#ifndef INC_10_WATCH_SERIALTASK_H
#define INC_10_WATCH_SERIALTASK_H
#include "cmsis_os2.h"

extern osMutexId_t SerialMutexHandle;   //用于管理串口资源
extern osSemaphoreId_t SerialBinarySemHandle;//用于管理串口收发是否完成

#endif //INC_10_WATCH_SERIALTASK_H