#ifndef __USER_TASKSINIT_H__
#define __USER_TASKSINIT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "cmsis_os.h"

    extern osThreadId_t HardwareInitTaskHandle;
    extern osThreadId_t LvHandlerTaskHandle;

void User_Tasks_Init(void);

#ifdef __cplusplus
}
#endif

#endif

