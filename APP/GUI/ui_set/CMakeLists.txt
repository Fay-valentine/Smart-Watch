SET(SOURCES screens/ui_HomePage.c
        screens/ui_MenuPage.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    fonts/ui_font_Font16.c
    fonts/ui_font_MenuIconfont.c)

add_library(ui ${SOURCES})
