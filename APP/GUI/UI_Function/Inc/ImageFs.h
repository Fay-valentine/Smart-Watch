//把W25Q64图片分区伪装成LVGL的文件系统盘 'S'
//路径格式: "S:0" "S:1" "S:2" 对应槽0/1/2(分区地址与ImgTransfer共用宏)
#ifndef INC_10_WATCH_IMAGEFS_H
#define INC_10_WATCH_IMAGEFS_H

#include <stdbool.h>
#include <stdint.h>

void ImgFs_Init(void);     //注册lv_fs驱动(lv_init之后、任何绘制之前调用)
void ImgFs_SelfTest(void); //开机验证:读S:0头4字节,应为 04 C0 03 23

//返回槽对应的LVGL路径常量("S:N"),越界返回NULL;字符串为static,可安全长期持有
const char * ImgFs_SlotPath(uint8_t slot);

//读槽头并校验(必须是TRUE_COLOR且w/h非零),用于串口命令判定"这槽有有效图"
bool ImgFs_SlotHasValidImage(uint8_t slot);

//解码槽图片的宽高(lv_img对FILE src不会自动定尺寸,控件必须显式set_size)
bool ImgFs_SlotSize(uint8_t slot, uint16_t * w, uint16_t * h);

#endif //INC_10_WATCH_IMAGEFS_H
