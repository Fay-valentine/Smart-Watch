#include "Pages.h"
#include "ui.h"

/* 页面注册表：新增页面时照抄一行即可 */
Page_t HomePage = {ui_HomePage_screen_init, ui_HomePage_screen_destroy, &ui_HomePage};
