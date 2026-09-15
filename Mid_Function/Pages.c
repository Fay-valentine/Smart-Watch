#include "Pages.h"
#include "ui.h"
#include "HomePage_RTC.h"


/* 页面注册表：新增页面时照抄一行即可 */
void HomePage_Init(void)
{
    ui_HomePage_screen_init();
    HomePage_RTC_Start();
}

void HomePage_Destroy(void)
{
    ui_HomePage_screen_destroy();
    HomePage_RTC_Stop();
}

Page_t HomePage = {HomePage_Init, HomePage_Destroy, &ui_HomePage};
