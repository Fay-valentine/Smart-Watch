//
// Created by a1191 on 2026/9/20.
//

#ifndef INC_10_WATCH_STOPWATCHPAGE_EVENT_H
#define INC_10_WATCH_STOPWATCHPAGE_EVENT_H
#include <stdint.h>
#include "lvgl.h"

typedef struct
{
    volatile uint8_t min;
    volatile uint8_t s;
    volatile uint8_t ms10;
    volatile uint8_t ms;
    volatile uint8_t flag;//0:停止刷新走时 1:开始刷新走时
}Stopwatch_t;

extern Stopwatch_t stopwatch;

void Stopwatch_TimeRefresh(void);
void StopwatchPage_TimerStop(void);
void StopwatchPage_Events_Init(void);
void ui_StopwatchPage_stopwatch_Init(void);

#endif //INC_10_WATCH_STOPWATCHPAGE_EVENT_H