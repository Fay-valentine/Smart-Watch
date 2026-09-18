//
// Created by a1191 on 2026/9/18.
//

#include "../Inc/ControlCenterPage_Event.h"

#include "HardWare_DataAccess.h"
#include "lvgl.h"
#include "Pages.h"
#include "screens/ui_ControlCenterPage.h"

static void ui_event_ControlCenterPage_cb(lv_event_t* e);

void ControlCenterPage_Events_Init(void)
{
    //进页面时用蓝牙真实状态回显按钮（按钮是CHECKABLE，点击会自动切换CHECKED）
    if (HWInterface.Bluetooth.GetState())
    {
        lv_obj_add_state(ui_BluetoothBTNInControl, LV_STATE_CHECKED);
    }
    else
    {
        lv_obj_clear_state(ui_BluetoothBTNInControl, LV_STATE_CHECKED);
    }

    //进页面时用背光真实状态回显滑条（生成代码里写死50，不回显会和实际亮度对不上）
    lv_slider_set_value(ui_BrightnessSlider, HWInterface.LCD.Get_Brightness(), LV_ANIM_OFF);

    lv_obj_add_event_cb(ui_SettingsBTNInControl,ui_event_ControlCenterPage_cb,LV_EVENT_CLICKED,&SettingsPage);
    lv_obj_add_event_cb(ui_BluetoothBTNInControl,ui_event_ControlCenterPage_cb,LV_EVENT_CLICKED,NULL);
    lv_obj_add_event_cb(ui_BrightnessSlider,ui_event_ControlCenterPage_cb,LV_EVENT_VALUE_CHANGED,NULL);
}


static void ui_event_ControlCenterPage_cb(lv_event_t* e)
{
    lv_obj_t* target=lv_event_get_target(e);

    if (target==ui_SettingsBTNInControl)
    {
        Page_Load(&SettingsPage);
    }
    else if (target==ui_BluetoothBTNInControl)
    {
        //跟随按钮的CHECKED状态做开/关联动
        bool on = lv_obj_has_state(target, LV_STATE_CHECKED);
        HWInterface.Bluetooth.Set(on);
    }

    else if (target==ui_BrightnessSlider)
    {
        uint8_t brightness = lv_slider_get_value(ui_BrightnessSlider);
        HWInterface.LCD.Set_Light(brightness < 5 ? 5 : brightness);
    }
}
