//
// Created by a1191 on 2026/9/15.
//

#include "../Inc/HomePage_Events.h"
#include "lvgl.h"
#include "PageManager.h"
#include "Pages.h"
#include "screens/ui_HomePage.h"
#include "screens/ui_MenuPage.h"

void ui_event_HomePage_cb(lv_event_t* e);

void HomePage_Events_Init(void)
{
    lv_obj_add_event_cb(ui_HomePage, ui_event_HomePage_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(ui_PhoneBar, LV_OBJ_FLAG_GESTURE_BUBBLE);// 恢复在进度条手势触发
}


/**
 * @brief 主页向右滑动进入菜单
 */
void ui_event_HomePage_cb(lv_event_t* e)
{
    lv_event_code_t code = lv_event_get_code(e);//事件代码
    lv_obj_t* target = lv_event_get_target(e);//事件触发对象

    if (code==LV_EVENT_GESTURE)
    {
        if (lv_indev_get_gesture_dir(lv_indev_get_act())==LV_DIR_RIGHT)//右滑
        {
            Page_Load(&MenuPage);
        }
    }
}
