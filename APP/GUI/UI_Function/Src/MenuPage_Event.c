//
// Created by a1191 on 2026/9/18.
//

#include "../Inc/MenuPage_Event.h"
#include "lvgl.h"
#include "PageManager.h"
#include "Pages.h"
#include "screens/ui_MenuPage.h"

// 菜单按钮 和 页面 映射表结构体
typedef struct {
    lv_obj_t ** btn;      // SquareLine 生成的控件指针
    Page_t    * page;
} MenuEntry_t;


static void ui_event_MenuPage_cb(lv_event_t* e);

// 菜单按钮 和 页面 映射表
static const MenuEntry_t menu_entries[] =
{
    {&ui_EnvironmentBTN, &EnvironmentPage},
    {&ui_StopwatchBTN, &StopwatchPage},
    {&ui_StepsBTN, &StepsCountPage},
    {&ui_MessageBTN, &MessagePage},
    {&ui_SettingsBTN, &SettingsPage},
    {&ui_AboutBTN, &AboutPage},
};


/**
 * @brief 通过 菜单按钮和页面映射表 循环注册每个按钮的点击事件
 */
void ui_event_MenuPage_Init(void)
{
    for (int i=0;i<sizeof(menu_entries)/sizeof(menu_entries[0]);++i)
    {
        if (menu_entries[i].btn == NULL || menu_entries[i].page == NULL)
        {
            continue;
        }
        // 注册按钮点击事件
        lv_obj_add_event_cb(*menu_entries[i].btn,
                            ui_event_MenuPage_cb,
                            LV_EVENT_CLICKED,
                            menu_entries[i].page);//可传递自定义指针，传入页面，从而能在回调函数访问该页面
    }
}


/**
 * @brief 加载 被点击按钮 对应的页面(事件注册已经筛选了事件类型，为点击)
 */
static void ui_event_MenuPage_cb(lv_event_t* e)
{
    Page_t* target_page=(Page_t*)lv_event_get_user_data(e);//获取的是 注册事件的 页面指针，即被点击按钮对应的页面
    if (target_page != NULL)
    {
        Page_Load(target_page);
    }
}