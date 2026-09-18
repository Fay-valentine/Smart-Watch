//
// Created by a1191 on 2026/9/18.
//

#include "Bluetooth.h"

extern osThreadId_t SerialTaskHandle;

static volatile bool bt_state = false;  //蓝牙开关唯一状态源

void Bluetooth_Set(bool on)
{
    if (bt_state == on) return;         //状态没变就不打扰任务
    bt_state = on;
    osThreadFlagsSet(SerialTaskHandle, BT_STATE_CHANGED_FLAG);//唤醒SerialTask立即处理
}

bool Bluetooth_GetState(void)
{
    return bt_state;
}
