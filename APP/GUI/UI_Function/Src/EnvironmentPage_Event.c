//
// Created by a1191 on 2026/10/2.
//

#include "../Inc/EnvironmentPage_Event.h"

#include <stdio.h>

#include "HardWare_DataAccess.h"
#include "lvgl.h"
#include "screens/ui_EnvironmentPage.h"

char msg[50];

void ui_event_EnvironmentPage_cb(lv_event_t* e);

void EnvironmentPage_Event_Init(void)
{
    lv_obj_add_event_cb(ui_EnvironmentPage, ui_event_EnvironmentPage_cb, LV_EVENT_VALUE_CHANGED, NULL);
}


void ui_event_EnvironmentPage_cb(lv_event_t* e)
{
    float temperature=0;
    float humidity=0;
    HWInterface.DHT20.Read(&temperature,&humidity);
    lv_bar_set_value(ui_TemperatureBar,(int32_t)temperature,LV_ANIM_OFF);
    lv_bar_set_value(ui_HumidityBar,(int32_t)humidity,LV_ANIM_OFF);
    sprintf(msg,"%.1f",temperature);
    lv_label_set_text(ui_TemperatureLabel,msg);
    sprintf(msg,"%.1f",humidity);
    lv_label_set_text(ui_HumidityLabel,msg);
}