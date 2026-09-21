//
// Created by a1191 on 2026/9/14.
//

#include "../Inc/HomePage_RTC.h"

#include <stdio.h>

#include "HardWare_DataAccess.h"
#include "PageManager.h"
#include "screens/ui_HomePage.h"


uint8_t ui_TimeHourValue = 15;
uint8_t ui_TimeMinuteValue = 59;

uint8_t ui_DateMonthValue = 11;
uint8_t ui_DateDayValue = 8;
uint8_t ui_DateWeekdayValue = 2;
const char * ui_Days[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

lv_timer_t* HomePage_RTC_Timer = NULL;

static void HomePage_RTC_Fresh_cb(lv_timer_t* timer);

/**
 * @brief 开启主页时间刷新
 */
void HomePage_RTC_Start(void)
{
    //使用哨兵，每次初始化首页，都对时间日期进行重绘
    //防止缓存的时间，日期value和RTC的相同（不会进行修改），而首页时间日期又被重建为写死的值
    ui_TimeHourValue    = 0xFF;  // 哨兵：与任何合法时/分/月/日(≤59/≤31)都不等
    ui_TimeMinuteValue  = 0xFF;
    ui_DateMonthValue   = 0xFF;
    ui_DateDayValue     = 0xFF;
    ui_DateWeekdayValue = 0xFF;
    if (HomePage_RTC_Timer==NULL)
    {
        HomePage_RTC_Timer=lv_timer_create(HomePage_RTC_Fresh_cb,500, NULL);
    }
}

/**
 * @brief 关闭主页时间刷新
 */
void HomePage_RTC_Stop(void)
{
    if (HomePage_RTC_Timer!=NULL)
    {
        lv_timer_del(HomePage_RTC_Timer);//删除首页时间刷新timer
        HomePage_RTC_Timer=NULL;
    }
}

/**
 * @brief 首页时间刷新 回调函数
 */
static void HomePage_RTC_Fresh_cb(lv_timer_t* timer)
{
    char value_str[10];
    if (Page_Get_NowPage()->page_obj==&ui_HomePage)//在主页时刷新时间
    {
        HW_DateTimeTypeDef DateTime;
        HWInterface.RealTimeClock.GetDateTime(&DateTime);

        //时
        if (DateTime.Hours!=ui_TimeHourValue)
        {
            ui_TimeHourValue=DateTime.Hours;
            sprintf(value_str,"%02d",ui_TimeHourValue);
            lv_label_set_text(ui_TimeHour,value_str);
        }

        //分
        if (DateTime.Minutes!=ui_TimeMinuteValue)
        {
            ui_TimeMinuteValue=DateTime.Minutes;
            sprintf(value_str,"%02d",ui_TimeMinuteValue);
            lv_label_set_text(ui_TimeMinute,value_str);
        }

        //月
        if (DateTime.Month!=ui_DateMonthValue)
        {
            ui_DateMonthValue=DateTime.Month;//月
            ui_DateDayValue=DateTime.Date;//日
            ui_DateWeekdayValue=DateTime.WeekDay;//星期

            sprintf(value_str,"%02d",ui_DateMonthValue);
            lv_label_set_text(ui_Month,value_str);

            sprintf(value_str,"%02d",ui_DateDayValue);
            lv_label_set_text(ui_Date,value_str);

            lv_label_set_text(ui_Week,ui_Days[ui_DateWeekdayValue-1]);
        }

        //日
        if (DateTime.Date!=ui_DateDayValue)
        {
            ui_DateDayValue=DateTime.Date;
            ui_DateWeekdayValue=DateTime.WeekDay;
            sprintf(value_str,"%02d",ui_DateDayValue);
            lv_label_set_text(ui_Date,value_str);
            lv_label_set_text(ui_Week,ui_Days[ui_DateWeekdayValue-1]);
        }
    }
}