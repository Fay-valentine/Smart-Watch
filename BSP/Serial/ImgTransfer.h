//
// 串口图片传输:imgopen握手 -> 二进制帧逐帧ACK -> close回读校验
//

#ifndef INC_10_WATCH_IMGTRANSFER_H
#define INC_10_WATCH_IMGTRANSFER_H

#include <stdint.h>
#include <stdbool.h>

//图片分区:槽i基址 = IMG_SLOT0_BASE + i * IMG_SLOT_STRIDE
//stride=256KB > 单图134404B,且4KB扇区对齐(页编程不跨页的前提)
#define IMG_SLOT0_BASE   0x101000UL
#define IMG_SLOT_STRIDE  0x40000UL
#define IMG_SLOT_MAX     3

#define IMG_DATA_FLAG    (1U<<1)   //唤醒SerialTask处理图像帧;BT_STATE_CHANGED_FLAG是1<<0

#define IMG_PAYLOAD_MAX  128       //不跨页:槽基址4KB对齐 + 偏移恒为128倍数
#define IMG_SEQ_CLOSE    0xFFFFU   //结束特殊帧的seq

extern volatile bool g_img_downloading;

void ImgTransfer_OpenCmd(const char* cmd);        //解析"imgopen <slot> <len> <crc>"
void ImgTransfer_OnRxISR(char* data, uint16_t size); //下载模式下ISR调用
void ImgTransfer_Pump(void);                        //任务上下文:解析帧+写Flash+回ACK
void ImgTransfer_Tick1s(void);                      //任务空闲每秒调用:超时中止检查

#endif //INC_10_WATCH_IMGTRANSFER_H
