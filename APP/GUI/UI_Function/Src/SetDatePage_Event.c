//
// Created by a1191 on 2026/9/20.
//

#include "../Inc/SetDatePage_Event.h"

#include "HardWare_DataAccess.h"
#include "lvgl.h"
#include "screens/ui_SetDatePage.h"
#include "HardWare_DataAccess.h"
#include "HomePage_RTC.h"
#include "PageManager.h"

void ui_event_SetDatePage_cb(lv_event_t* e);

/**
 * @brief 注册按键 设置日期；并通过当前RTC日期，将滑动框定位到当前日期
 */
void SetDatePage_Events_Init(void)
{
    lv_obj_add_event_cb(ui_ConfirmBTNInDate, ui_event_SetDatePage_cb, LV_EVENT_CLICKED, NULL);

    HW_DateTimeTypeDef now_date;
    HWInterface.RealTimeClock.GetDateTime(&now_date);   // now.Year 是两位年份，如 26

    lv_roller_set_selected(ui_YearRoller,  now_date.Year - 20,   LV_ANIM_OFF);
    lv_roller_set_selected(ui_MonthRoller, now_date.Month - 1,   LV_ANIM_OFF);
    lv_roller_set_selected(ui_DayRoller,   now_date.Date  - 1,   LV_ANIM_OFF);
}

/**
 * @brief 获取日期设置选中的值，在点击OK按键后设置RTC日期
 */
void ui_event_SetDatePage_cb(lv_event_t* e)
{
    //获取选中的索引
    //年 = 20 + 下标（得到两位年份 20~27），月/日 = 下标 + 1
    uint8_t set_year=lv_roller_get_selected(ui_YearRoller)+20;
    uint8_t set_month=lv_roller_get_selected(ui_MonthRoller)+1;
    uint8_t set_day=lv_roller_get_selected(ui_DayRoller)+1;

    //设置RTC日期
    HWInterface.RealTimeClock.SetDate(set_year, set_month, set_day);

    Page_Back();//设置完成后退出页面
}
