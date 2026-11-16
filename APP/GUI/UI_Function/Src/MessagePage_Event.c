//
// Created by a1191 on 2026/9/22.
//

#include "../Inc/MessagePage_Event.h"
#include "lvgl.h"
#include "screens/ui_MessagePage.h"
#include <string.h>

/**
 * @brief 将消息收件箱中的消息 刷新到 消息窗口
 */
void MessagePage_render(const MessagePage_Inbox_t* messagePage_inbox)
{
    /* 存"指向全局变量的指针"而非控件地址本身：
   页面销毁重建后，透过它读到的永远是最新的控件指针（同 MenuEntry_t 的技巧） */
    lv_obj_t **labels[MSG_WINDOWS_NUM] = { &ui_Message1,&ui_Message2,&ui_Message3,&ui_Message4,&ui_Message5 };

    //将消息刷新到消息窗口
    for (int i = 0; i < MSG_WINDOWS_NUM; ++i)
    {
        //显示msg_num个窗口，并检测字符串不为空
        if (i<messagePage_inbox->msg_num && messagePage_inbox->msg_list[i].text[0]!='\0')
        {
            lv_label_set_text(*labels[i], messagePage_inbox->msg_list[i].text);     // 放消息
            //lv_obj_clear_flag(*btns[i], LV_OBJ_FLAG_HIDDEN);   // 显示窗
        }
        else
        {
            lv_label_set_text(*labels[i], "");                 // 删消息
            //lv_obj_add_flag(*btns[i], LV_OBJ_FLAG_HIDDEN);      // 隐藏窗
        }
    }
}