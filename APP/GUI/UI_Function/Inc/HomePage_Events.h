//
// Created by a1191 on 2026/9/15.
//

#ifndef INC_10_WATCH_HOMEPAGE_EVENTS_H
#define INC_10_WATCH_HOMEPAGE_EVENTS_H




#include <stdbool.h>
#include <stdint.h>

//LvHandlerTask的flag位(0/2/3已被PAGE_BACK/MSG/WAKE占用,这里取1<<4)
#define BG_SWITCH_FLAG (1U<<4)

void HomePage_Events_Init(void);

//以下三个函数构成"串口请求 -> GUI任务应用"的切换闭环:
bool HomePage_RequestBg(uint8_t slot);      //任意任务可调:记槽号+唤醒GUI任务
void HomePage_ApplyPendingBg(void);         //仅LvHandlerTask调用:真正改img控件src
void HomePage_Events_BgReset(void);         //HomePage_Destroy调用:控件将随屏幕auto_del,置空指针

#endif //INC_10_WATCH_HOMEPAGE_EVENTS_H