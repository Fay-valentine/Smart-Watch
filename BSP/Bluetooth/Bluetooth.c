//
// Created by a1191 on 2026/9/18.
//

#include "Bluetooth.h"

extern osThreadId_t SerialTaskHandle;

static volatile bool bt_state = false;  //蓝牙开关状态

/**
 * @brief 设置蓝牙开关状态
 * @param on true: 蓝牙开，false: 蓝牙关
 */
void Bluetooth_Set(bool on)
{
    if (bt_state == on) return;         //状态没变就不打扰任务
    bt_state = on;
    osThreadFlagsSet(SerialTaskHandle, BT_STATE_CHANGED_FLAG);//唤醒SerialTask立即处理
}

/**
 * @return true: 蓝牙开，false: 蓝牙关
 */
bool Bluetooth_GetState(void)
{
    return bt_state;
}
