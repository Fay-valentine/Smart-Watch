//
// Created by a1191 on 2026/9/15.
//

#include "../Inc/HomePage_Events.h"
#include "lvgl.h"
#include "PageManager.h"
#include "Pages.h"
#include "screens/ui_HomePage.h"
#include "screens/ui_MenuPage.h"
#include "cmsis_os2.h"
#include "user_TasksInit.h"
#include "ImgTransfer.h"
#include "ImageFs.h"

static void ui_event_HomePage_cb(lv_event_t* e);

//首页背景图:数据来自W25Q64图片分区(lv_fs盘'S'),逐行流式读取
//bg_img仅GUI任务可见;跨线程只有一个volatile槽号要传,所以请求/应用分离
#define BG_SLOT_NONE 0xFF
static lv_obj_t * bg_img;
static volatile uint8_t bg_pending = BG_SLOT_NONE; //无请求
static uint8_t bg_applied = BG_SLOT_NONE;          //最近一次已应用的槽号(仅GUI任务读写)

void HomePage_Events_Init(void)
{
    //此处仍在Page_Init的vTaskSuspendAll区间内:不能set_src(会经decoder_get_info读Flash拿
    //互斥锁=死锁,且'S'盘此刻尚未注册),只建控件,src由GUI任务第一次tick的ApplyPendingBg补投。
    bg_img = lv_img_create(ui_HomePage);
    bg_pending = (bg_applied == BG_SLOT_NONE) ? 0 : bg_applied; //开机默认槽0;重进页面保持上次选择
    lv_obj_set_size(bg_img, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL)); //占位尺寸,Apply时按图片头校正
    lv_obj_center(bg_img);
    lv_obj_move_background(bg_img);              //垫到底层,时间/电量控件压其上
    lv_obj_add_flag(bg_img, LV_OBJ_FLAG_GESTURE_BUBBLE);   //别吃掉右滑手势(ui_PhoneBar同款教训)
    lv_obj_clear_flag(bg_img, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_add_event_cb(ui_HomePage, ui_event_HomePage_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_flag(ui_PhoneBar, LV_OBJ_FLAG_GESTURE_BUBBLE);// 恢复在进度条手势触发
}

bool HomePage_RequestBg(uint8_t slot)
{
    if (slot >= IMG_SLOT_MAX) return false;
    bg_pending = slot;
    osThreadFlagsSet(LvHandlerTaskHandle, BG_SWITCH_FLAG); //切换必须回GUI任务执行,LVGL非线程安全
    return true;
}

void HomePage_ApplyPendingBg(void)
{
    uint8_t s = bg_pending;
    if (s == BG_SLOT_NONE) return;      //无请求(可能已被上次apply消费)
    bg_pending = BG_SLOT_NONE;
    bg_applied = s;
    if (bg_img)   //页面可能不在首页(控件已随屏幕销毁)->只记槽号,Init时生效
    {
        lv_img_set_src(bg_img, ImgFs_SlotPath(s));
        uint16_t iw, ih;
        if (ImgFs_SlotSize(s, &iw, &ih)) lv_obj_set_size(bg_img, iw, ih); //按图片头校正真实尺寸
    }
}

void HomePage_Events_BgReset(void)
{
    bg_img = NULL; //lv_scr_load_anim(auto_del)随后会删除ui_HomePage整棵树,此处只断引用
}


/**
 * @brief 主页向右滑动进入菜单
 */
static void ui_event_HomePage_cb(lv_event_t* e)
{
    lv_dir_t direction = lv_indev_get_gesture_dir(lv_indev_get_act()); //获取手势方向

    if (direction==LV_DIR_RIGHT)//右滑进入菜单
    {
        Page_Load(&MenuPage);
    }
    else if (direction==LV_DIR_BOTTOM)//下滑进入控制面板
    {
        Page_Load(&ControlCenterPage);
    }
}
