#include "Pages.h"
#include "ui.h"
#include "HomePage_RTC.h"
#include "HomePage_Events.h"
#include "MenuPage_Event.h"
#include "SettingsPage_Event.h"
#include "ControlCenterPage_Event.h"
#include "SetDatePage_Event.h"
#include "SetTimePage_Event.h"
#include "StopwatchPage_Event.h"

/* 页面注册表：新增页面时照抄一行即可 */


//主页
void HomePage_Init(void)
{
    ui_HomePage_screen_init();
    HomePage_Events_Init();//注册滑动事件
    HomePage_RTC_Start();
}

void HomePage_Destroy(void)
{
    HomePage_RTC_Stop();
}

Page_t HomePage = {HomePage_Init, HomePage_Destroy, &ui_HomePage};


//菜单页
void MenuPage_Init(void)
{
    ui_MenuPage_screen_init();
    ui_event_MenuPage_Init();
}

void MenuPage_Destroy(void)
{

}

Page_t MenuPage = {MenuPage_Init, MenuPage_Destroy, &ui_MenuPage};


//控制面板页
void ControlCenterPage_Init(void)
{
    ui_ControlCenterPage_screen_init();
    ControlCenterPage_Events_Init();
}

void ControlCenterPage_Destroy(void)
{

}

Page_t ControlCenterPage = {ControlCenterPage_Init, ControlCenterPage_Destroy, &ui_ControlCenterPage};


//秒表页
void StopwatchPage_Init(void)
{
    ui_StopwatchPage_screen_init();
    ui_StopwatchPage_stopwatch_Init();
    StopwatchPage_Events_Init();
}

void StopwatchPage_Destroy(void)
{
    StopwatchPage_TimerStop();
}

Page_t StopwatchPage = {StopwatchPage_Init, StopwatchPage_Destroy, &ui_StopwatchPage};


//环境页
void EnvironmentPage_Init(void)
{
    ui_EnvironmentPage_screen_init();
}

void EnvironmentPage_Destroy(void)
{

}

Page_t EnvironmentPage = {EnvironmentPage_Init, EnvironmentPage_Destroy, &ui_EnvironmentPage};


//计步页
void StepsCountPage_Init(void)
{
    ui_StepsCountPage_screen_init();
}

void StepsCountPage_Destroy(void)
{

}

Page_t StepsCountPage = {StepsCountPage_Init, StepsCountPage_Destroy, &ui_StepsCountPage};


//设置页
void SettingsPage_Init(void)
{
    ui_SettingsPage_screen_init();
    ui_event_SettingsPage_Init();
}

void SettingsPage_Destroy(void)
{

}

Page_t SettingsPage = {SettingsPage_Init, SettingsPage_Destroy, &ui_SettingsPage};


//日期设置页
void SetDatePage_Init(void)
{
    ui_SetDatePage_screen_init();
    SetDatePage_Events_Init();
}

void SetDatePage_Destroy(void)
{

}

Page_t SetDatePage = {SetDatePage_Init, SetDatePage_Destroy, &ui_SetDatePage};


//时间设置页
void SetTimePage_Init(void)
{
    ui_SetTimePage_screen_init();
    SetTimePage_Events_Init();
}

void SetTimePage_Destroy(void)
{

}

Page_t SetTimePage = {SetTimePage_Init, SetTimePage_Destroy, &ui_SetTimePage};


//息屏页
void ScreenOffPage_Init(void)
{
    ui_ScreenOffPage_screen_init();
}

void ScreenOffPage_Destroy(void)
{

}

Page_t ScreenOffPage = {ScreenOffPage_Init, ScreenOffPage_Destroy, &ui_ScreenOffPage};


//关于页
void AboutPage_Init(void)
{
    ui_AboutPage_screen_init();
}

void AboutPage_Destroy(void)
{

}

Page_t AboutPage = {AboutPage_Init, AboutPage_Destroy, &ui_AboutPage};


//消息页
void MessagePage_Init(void)
{
    ui_MessagePage_screen_init();
}

void MessagePage_Destroy(void)
{

}

Page_t MessagePage = {MessagePage_Init, MessagePage_Destroy, &ui_MessagePage};
