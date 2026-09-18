SET(SOURCES screens/ui_HomePage.c
    screens/ui_MenuPage.c
    screens/ui_StopwatchPage.c
    screens/ui_EnvironmentPage.c
    screens/ui_StepsCountPage.c
    screens/ui_SettingsPage.c
    screens/ui_ControlCenterPage.c
    screens/ui_SetDatePage.c
    screens/ui_SetTimePage.c
    screens/ui_ScreenOffPage.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    fonts/ui_font_BrightnessIconfont.c
    fonts/ui_font_EnvironmentIconfont32.c
    fonts/ui_font_Font16.c
    fonts/ui_font_Font20.c
    fonts/ui_font_Font22.c
    fonts/ui_font_Font38.c
    fonts/ui_font_Font40.c
    fonts/ui_font_Font48.c
    fonts/ui_font_MenuIconfont.c
    fonts/ui_font_SettingsIconfont32.c
    fonts/ui_font_StepsCountIconfont64.c
    fonts/ui_font_StopwatchIconfont32.c)

add_library(ui ${SOURCES})
