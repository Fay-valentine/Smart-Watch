//
// Created by a1191 on 2026/10/26.
//

#include "StepsCountPage_Event.h"
#include <stdint.h>
#include <stdio.h>
#include "HardWare_DataAccess.h"
#include "lvgl.h"
#include "screens/ui_StepsCountPage.h"

uint32_t ui_StepsCountLabel_value = 4500;

lv_timer_t* StepsCountPage_Timer=NULL;

void ui_event_StepsCountPage_Refresh_cb(lv_timer_t* timer);

void StepsCountPage_Start_Refresh(void)
{
    if (StepsCountPage_Timer==NULL)
    {
        StepsCountPage_Timer=lv_timer_create(ui_event_StepsCountPage_Refresh_cb,500, NULL);
    }
}

void ui_event_StepsCountPage_Refresh_cb(lv_timer_t* timer)
{
    if (HWInterface.MPU6050.steps != ui_StepsCountLabel_value)
    {
        lv_label_set_text_fmt(ui_nowStepLabel, "%lu", HWInterface.MPU6050.steps);
        ui_StepsCountLabel_value=HWInterface.MPU6050.steps;
        lv_arc_set_value(ui_StepsArc, (int16_t)(HWInterface.MPU6050.steps>10000?10000:HWInterface.MPU6050.steps));
    }
}

void StepsCountPage_Stop_Refresh(void)
{
    if (StepsCountPage_Timer!=NULL)
    {
        lv_timer_del(StepsCountPage_Timer);
        StepsCountPage_Timer=NULL;
    }
}
