//
// Created by a1191 on 2026/9/24.
//

#ifndef INC_10_WATCH_IDLEMEASURE_TASK_H
#define INC_10_WATCH_IDLEMEASURE_TASK_H


#include <stdbool.h>
#include <stdint.h>

#define IDLE_ENTRY_FLAG     (1U<<0)
#define IDLE_EXIT_FLAG     (1U<<1)

#define SLEEP_ENTRY_FLAG     (1U<<0)

extern uint16_t IdleTimerCount_100ms;

void Idle_Monitor_Switch(bool switch_on);
void Idle_Monitor_Set_LightOff_Time(uint16_t lightOff_time_100ms);

#endif //INC_10_WATCH_IDLEMEASURE_TASK_H