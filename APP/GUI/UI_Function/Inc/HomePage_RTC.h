//
// Created by a1191 on 2026/9/14.
//

#ifndef INC_10_WATCH_HOMEPAGE_RTC_H
#define INC_10_WATCH_HOMEPAGE_RTC_H
#include "misc/lv_timer.h"

extern lv_timer_t* HomePage_RTC_Timer;

void HomePage_RTC_Start(void);
void HomePage_RTC_Stop(void);

#endif //INC_10_WATCH_HOMEPAGE_RTC_H