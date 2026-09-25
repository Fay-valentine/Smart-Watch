//
// Created by a1191 on 2026/9/25.
//

#include "../Inc/ScreenOffPage_Event.h"

#include "lvgl.h"
#include "screens/ui_ScreenOffPage.h"
#include <string.h>
#include "IdleMeasure_Task.h"
#include "PageManager.h"

void ui_event_ScreenOffPage_cb(lv_event_t* e);

void ScreenOffPage_Event_Init(void)
{
    lv_obj_add_event_cb(ui_ConfirmBTNInOff,ui_event_ScreenOffPage_cb,LV_EVENT_CLICKED,NULL);
}


void ui_event_ScreenOffPage_cb(lv_event_t* e)
{
    char select[10];
    lv_roller_get_selected_str(ui_ScreenOffRoller,select,sizeof(select));

    if (strcmp(select,"15s")==0)
    {
        Idle_Monitor_Set_LightOff_Time(150);
    }
    else if (strcmp(select,"30s")==0)
    {
        Idle_Monitor_Set_LightOff_Time(300);
    }
    else if (strcmp(select,"60s")==0)
    {
        Idle_Monitor_Set_LightOff_Time(600);
    }
    else if (strcmp(select,"2min")==0)
    {
        Idle_Monitor_Set_LightOff_Time(1200);
    }
    else if (strcmp(select,"5min")==0)
    {
        Idle_Monitor_Set_LightOff_Time(3000);
    }
    else if (strcmp(select,"10min")==0)
    {
        Idle_Monitor_Set_LightOff_Time(6000);
    }
    else if (strcmp(select,"30min")==0)
    {
        Idle_Monitor_Set_LightOff_Time(18000);
    }
    else if (strcmp(select,"1hour")==0)
    {
        Idle_Monitor_Set_LightOff_Time(36000);
    }
    else if (strcmp(select,"never")==0)
    {
        Idle_Monitor_Switch(false);
    }
    Page_Back();//设置完成后退出页面
}
