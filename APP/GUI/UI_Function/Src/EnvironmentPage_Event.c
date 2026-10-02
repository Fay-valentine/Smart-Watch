//
// Created by a1191 on 2026/10/2.
//

#include "../Inc/EnvironmentPage_Event.h"

#include <stdio.h>

#include "HardWare_DataAccess.h"
#include "lvgl.h"
#include "screens/ui_EnvironmentPage.h"

float ui_temperatureLabel_value=26;
float ui_humidityLabel_value=40;

lv_timer_t* EnvironmentPage_Timer=NULL;

void ui_event_EnvironmentPage_Refresh_cb(lv_timer_t* timer);

void EnvironmentPage_Start_Refresh(void)
{
    if (EnvironmentPage_Timer==NULL)
    {
        EnvironmentPage_Timer=lv_timer_create(ui_event_EnvironmentPage_Refresh_cb,500, NULL);
    }
}


void ui_event_EnvironmentPage_Refresh_cb(lv_timer_t* timer)
{
    if (HWInterface.DHT20.temperature != 0.0 && HWInterface.DHT20.temperature != ui_temperatureLabel_value)
    {
        lv_label_set_text_fmt(ui_TemperatureLabel, "%.1f", HWInterface.DHT20.temperature);
        ui_temperatureLabel_value=HWInterface.DHT20.temperature;
        lv_bar_set_value(ui_TemperatureBar, (int32_t)HWInterface.DHT20.temperature, LV_ANIM_ON);
    }
    if (HWInterface.DHT20.humidity != 0.0 && HWInterface.DHT20.humidity != ui_humidityLabel_value)
    {
        lv_label_set_text_fmt(ui_HumidityLabel, "%.1f", HWInterface.DHT20.humidity);
        ui_humidityLabel_value=HWInterface.DHT20.humidity;
        lv_bar_set_value(ui_HumidityBar, (int32_t)HWInterface.DHT20.humidity, LV_ANIM_ON);
    }
}

void EnvironmentPage_Stop_Refresh(void)
{
    if (EnvironmentPage_Timer!=NULL)
    {
        lv_timer_del(EnvironmentPage_Timer);
        EnvironmentPage_Timer=NULL;
    }
}