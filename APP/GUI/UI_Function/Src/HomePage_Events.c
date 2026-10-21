//
// Created by a1191 on 2026/9/15.
//

#include "../Inc/HomePage_Events.h"
#include "lvgl.h"
#include "PageManager.h"
#include "Pages.h"
#include "screens/ui_HomePage.h"
#include "screens/ui_MenuPage.h"

static void ui_event_HomePage_cb(lv_event_t* e);

void HomePage_Events_Init(void)
{
    lv_obj_add_event_cb(ui_HomePage, ui_event_HomePage_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(ui_PhoneBar, LV_OBJ_FLAG_GESTURE_BUBBLE);// 恢复在进度条手势触发
}


/**
 * @brief 主页向右滑动进入菜单
 */
static void ui_event_HomePage_cb(lv_event_t* e)
{
    lv_dir_t direction = lv_indev_get_gesture_dir(lv_indev_get_act()); //获取手势方向

    if (direction==LV_DIR_RIGHT)//右滑进入菜单
    {
        Page_Load(&MenuPage);
    }
    else if (direction==LV_DIR_BOTTOM)//下滑进入控制面板
    {
        Page_Load(&ControlCenterPage);
    }
}
