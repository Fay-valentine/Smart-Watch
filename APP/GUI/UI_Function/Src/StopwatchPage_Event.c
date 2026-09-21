//
// Created by a1191 on 2026/9/20.
//

#include "../Inc/StopwatchPage_Event.h"

#include <stdio.h>

#include "screens/ui_StopwatchPage.h"

lv_timer_t* stopwatch_timer=NULL;
Stopwatch_t stopwatch={0,0,0,0,0};
static void ui_event_StopwatchPage_cb(lv_event_t* e);
static void StopwatchPage_Refresh_cb(lv_timer_t* timer);


void StopwatchPage_Events_Init(void)
{
    stopwatch_timer=lv_timer_create(StopwatchPage_Refresh_cb,10,NULL);//约10ms刷新一次
    lv_timer_pause(stopwatch_timer);//先暂停timer

    lv_obj_add_event_cb(ui_StartStopBTN, ui_event_StopwatchPage_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_ResetStopwacthBTN, ui_event_StopwatchPage_cb, LV_EVENT_CLICKED, NULL);

}


void StopwatchPage_TimerStop(void)
{
    stopwatch.flag=0;
    if (stopwatch_timer!=NULL)
    {
        lv_timer_del(stopwatch_timer);
        stopwatch_timer=NULL;
    }
}


/**
 * @brief 秒表时间累加
 */
void Stopwatch_TimeRefresh(void)
{
    if(stopwatch.flag==1)
    {
        stopwatch.ms++;
        if (stopwatch.ms>=10)
        {
            stopwatch.ms=0;
            stopwatch.ms10++;
        }
        if (stopwatch.ms10>=100)
        {
            stopwatch.ms10=0;
            stopwatch.s++;
        }
        if (stopwatch.s>=60)
        {
            stopwatch.s=0;
            stopwatch.min++;
        }
        if (stopwatch.min>=60)
        {
            stopwatch.min=0;
        }
    }
}


/**
 * @brief 秒表时间初始化
 */
void ui_StopwatchPage_stopwatch_Init(void)
{
    stopwatch.flag=0;
    stopwatch.min=0;
    stopwatch.s=0;
    stopwatch.ms10=0;
    stopwatch.ms=0;
}


/**
 * @brief 刷新秒表的时间标签
 */
static void StopwatchPage_Refresh_cb(lv_timer_t* timer)
{
    char label_str_buf[5];

    //分钟
    sprintf(label_str_buf,"%02d",stopwatch.min);
    lv_label_set_text(ui_MinutesLabel,label_str_buf);
    //秒
    sprintf(label_str_buf,"%02d",stopwatch.s);
    lv_label_set_text(ui_SecondsLabel,label_str_buf);
    //毫秒
    sprintf(label_str_buf,"%02d",stopwatch.ms10);
    lv_label_set_text(ui_MilisecondsLabel,label_str_buf);
}


/**
 * @brief 秒表页面处理回调，按下开始按键，开始计时，按下暂停按键，停止计时，按下重置按键，重置计时
 */
static void ui_event_StopwatchPage_cb(lv_event_t* e)
{
    lv_obj_t* target=lv_event_get_target(e);

    if (target==ui_StartStopBTN && lv_obj_has_state(target,LV_STATE_CHECKED))//开始计时
    {
        stopwatch.flag=1;
        lv_timer_resume(stopwatch_timer);
    }
    else if (target==ui_StartStopBTN && !lv_obj_has_state(target,LV_STATE_CHECKED))//停止计时
    {
        stopwatch.flag=0;
        StopwatchPage_Refresh_cb(stopwatch_timer);  //先刷新最后一帧
        lv_timer_pause(stopwatch_timer);
    }
    else if (target==ui_ResetStopwacthBTN)//重置计时
    {
        stopwatch.flag=0;
        lv_timer_pause(stopwatch_timer);

        ui_StopwatchPage_stopwatch_Init();//重置时间变量

        lv_obj_clear_state(ui_StartStopBTN, LV_STATE_CHECKED);//清除开始暂停键的状态

        //清状态不会触发CLICKED，需要手动把图标换回"开始"
        lv_label_set_text(ui_StartIconfont, "");  //U+E87A 开始图标

        //重置时间标签
        lv_label_set_text(ui_MinutesLabel,"00");
        lv_label_set_text(ui_SecondsLabel,"00");
        lv_label_set_text(ui_MilisecondsLabel,"00");
    }
}

