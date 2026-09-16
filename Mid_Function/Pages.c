#include "Pages.h"
#include "ui.h"
#include "HomePage_RTC.h"
#include "HomePage_Events.h"

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
}

void MenuPage_Destroy(void)
{

}

Page_t MenuPage = {MenuPage_Init, MenuPage_Destroy, &ui_MenuPage};
