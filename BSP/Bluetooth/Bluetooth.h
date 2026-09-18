//
// Created by a1191 on 2026/9/18.
//

#ifndef INC_10_WATCH_BLUETOOTH_H
#define INC_10_WATCH_BLUETOOTH_H

#include "cmsis_os2.h"
#include <stdbool.h>
//使用有线串口模拟蓝牙透传，一样的

#define BT_STATE_CHANGED_FLAG    (1U<<0)   //仅作唤醒SerialTask的信号，不表示开/关状态

void Bluetooth_Set(bool on);                //任何线程/ISR均可调用
bool Bluetooth_GetState(void);              //volatile bool读取，ISR安全

#endif //INC_10_WATCH_BLUETOOTH_H
