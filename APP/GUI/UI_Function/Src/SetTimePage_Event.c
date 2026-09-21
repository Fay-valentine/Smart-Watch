//
// Created by a1191 on 2026/9/21.
//

#include "../Inc/SetTimePage_Event.h"

#include "HardWare_DataAccess.h"
#include "HomePage_RTC.h"
#include "lvgl.h"
#include "PageManager.h"
#include "screens/ui_SetTimePage.h"

void ui_event_SetTimePage_cb(lv_event_t* e);

/**
 * @brief 注册按键 设置时间；并通过当前RTC时间，将滑动框定位到当前时间
 */
void SetTimePage_Events_Init(void)
{
    lv_obj_add_event_cb(ui_ConfirmBTNInTime, ui_event_SetTimePage_cb, LV_EVENT_CLICKED, NULL);

    HW_DateTimeTypeDef now_time;
    HWInterface.RealTimeClock.GetDateTime(&now_time);

    lv_roller_set_selected(ui_HourRoller, now_time.Hours, LV_ANIM_OFF);
    lv_roller_set_selected(ui_MinuteRoller, now_time.Minutes, LV_ANIM_OFF);
}


/**
 * @brief 获取时间设置选中的值，在点击OK按键后设置RTC时间
 */
void ui_event_SetTimePage_cb(lv_event_t* e)
{
    //获取选中的索引
    //下标值和选中的值相同
    uint8_t set_hour=lv_roller_get_selected(ui_HourRoller);
    uint8_t set_minute=lv_roller_get_selected(ui_MinuteRoller);

    //设置RTC时间
    HWInterface.RealTimeClock.SetTime(set_hour, set_minute,0);

    Page_Back();//设置完成后退出页面
}