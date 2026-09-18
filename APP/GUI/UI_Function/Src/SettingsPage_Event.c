//
// Created by a1191 on 2026/9/18.
//

#include "../Inc/SettingsPage_Event.h"

#include "PageManager.h"
#include "core/lv_event.h"
#include "screens/ui_SettingsPage.h"
#include "Pages.h"

typedef struct {
    lv_obj_t ** btn;
    Page_t    * page;
} Settings_Entry_t;

Settings_Entry_t settings_entries[] =
{
    {&ui_SetTimeBTN,&SetTimePage},
    {&ui_SetDateBTN,&SetDatePage},
    {&ui_ScreenOffBTN,&ScreenOffPage},
};

static void ui_event_SettingsPage_cb(lv_event_t* e);

void ui_event_SettingsPage_Init(void)
{
    for (int i=0;i<sizeof(settings_entries)/sizeof(settings_entries[0]);++i)
    {
        if (settings_entries[i].btn == NULL || settings_entries[i].page == NULL)
        {
            continue;
        }
        lv_obj_add_event_cb(*settings_entries[i].btn,
                            ui_event_SettingsPage_cb,
                            LV_EVENT_CLICKED,
                            settings_entries[i].page);
    }
}


static void ui_event_SettingsPage_cb(lv_event_t* e)
{
    Page_t* target_page=(Page_t*)lv_event_get_user_data(e);
    if (target_page != NULL)
    {
        Page_Load(target_page);
    }
}
