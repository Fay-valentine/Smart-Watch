#ifndef INC_10_WATCH_PAGES_H
#define INC_10_WATCH_PAGES_H

#include "PageManager.h"

/*
 * 所有接入页面栈的页面的 Page_t 实例统一在这里声明。
 * 实例定义放在 Pages.c（非 SquareLine 生成文件），
 * 这样重新导出 UI 文件覆盖 screens 目录下的 .c/.h 时页面注册不会丢失。
 */
extern Page_t HomePage;
extern Page_t MenuPage;
extern Page_t ControlCenterPage;
extern Page_t StopwatchPage;
extern Page_t EnvironmentPage;
extern Page_t StepsCountPage;
extern Page_t SettingsPage;
extern Page_t SetDatePage;
extern Page_t SetTimePage;
extern Page_t ScreenOffPage;
extern Page_t AboutPage;
extern Page_t MessagePage;

#endif //INC_10_WATCH_PAGES_H
