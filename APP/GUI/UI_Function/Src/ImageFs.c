//
// W25Q64图片分区 -> LVGL文件系统盘'S'
// LVGL绘制img控件时逐行调用read/seek,数据现场从SPI读,RAM里永远只有一行像素
//

#include "ImageFs.h"

#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"
#include "W25Q64.h"
#include "ImgTransfer.h"      //IMG_SLOT0_BASE/STRIDE/MAX:分区布局唯一定义处
#include "Serial_command.h"

typedef struct {
    uint32_t base;   //该图在W25Q64中的绝对起始地址
    uint32_t pos;    //当前读指针(相对文件开头)
    bool     used;
} ImgFsCtx;

#define IMGFS_HANDLE_POOL 4   //LVGL图片缓存会同时持有若干打开的文件,留余量

static lv_fs_drv_t fs_drv;                 //必须static:注册进链表的是它的地址
static ImgFsCtx ctx_pool[IMGFS_HANDLE_POOL];

//必须带.bin扩展名:内置解码器 lv_img_decoder_built_in_info 用 strcmp(get_ext,"bin") 硬校验
static const char * slot_paths[IMG_SLOT_MAX] = {"S:0.bin", "S:1.bin", "S:2.bin"};

/* ---------------- lv_fs 回调:由LVGL绘制路径(GUI任务)调用 ---------------- */

static void * ImgFs_open(lv_fs_drv_t * drv, const char * path, lv_fs_mode_t mode)
{
    //LVGL已剥掉"S:",path是"0.bin"/"1.bin"/"2.bin"(.bin为内置解码器硬要求);非法或句柄池满 -> NULL
    if (path[0] < '0' || path[0] >= '0' + IMG_SLOT_MAX) return NULL;

    for (int i = 0; i < IMGFS_HANDLE_POOL; i++)
    {
        if (!ctx_pool[i].used)
        {
            ctx_pool[i].used = true;
            ctx_pool[i].base = IMG_SLOT0_BASE + (uint32_t)(path[0] - '0') * IMG_SLOT_STRIDE;
            ctx_pool[i].pos  = 0;
            return &ctx_pool[i];
        }
    }
    return NULL;
}

static lv_fs_res_t ImgFs_close(lv_fs_drv_t * drv, void * file_p)
{
    ImgFsCtx * h = (ImgFsCtx *)file_p;
    if (h < &ctx_pool[0] || h >= &ctx_pool[IMGFS_HANDLE_POOL]) return LV_FS_RES_INV_PARAM;
    h->used = false;
    return LV_FS_RES_OK;
}

static lv_fs_res_t ImgFs_read(lv_fs_drv_t * drv, void * file_p,
                              void * buf, uint32_t btr, uint32_t * br)
{
    ImgFsCtx * h = (ImgFsCtx *)file_p;
    uint32_t room = IMG_SLOT_STRIDE - h->pos;          //不越出本槽
    uint32_t n = (btr < room) ? btr : room;
    if (n == 0) { *br = 0; return LV_FS_RES_FS_ERR; }

    //W25Q64内部互斥锁保护,与串口写图天然互斥
    if (!W25Q64_Read_Data(h->base + h->pos, (uint8_t *)buf, n)) return LV_FS_RES_HW_ERR;

    h->pos += n;
    *br = n;
    return LV_FS_RES_OK;
}

static lv_fs_res_t ImgFs_seek(lv_fs_drv_t * drv, void * file_p, uint32_t pos, lv_fs_whence_t whence)
{
    ImgFsCtx * h = (ImgFsCtx *)file_p;
    int32_t np;
    switch (whence)
    {
        case LV_FS_SEEK_SET: np = (int32_t)pos;                                    break;
        case LV_FS_SEEK_CUR: np = (int32_t)h->pos + (int32_t)pos;                  break;
        case LV_FS_SEEK_END: np = (int32_t)IMG_SLOT_STRIDE + (int32_t)pos; break; //图片解码只用SET/CUR,END按槽长近似
        default: return LV_FS_RES_INV_PARAM;
    }
    if (np < 0 || (uint32_t)np > IMG_SLOT_STRIDE) return LV_FS_RES_INV_PARAM;
    h->pos = (uint32_t)np;      //只动游标:SIP NOR随机读没有"定位成本"
    return LV_FS_RES_OK;
}

static lv_fs_res_t ImgFs_tell(lv_fs_drv_t * drv, void * file_p, uint32_t * pos_p)
{
    *pos_p = ((ImgFsCtx *)file_p)->pos;
    return LV_FS_RES_OK;
}

/* ---------------- 对外接口 ---------------- */

void ImgFs_Init(void)
{
    lv_fs_drv_init(&fs_drv);
    fs_drv.letter   = 'S';
    fs_drv.open_cb  = ImgFs_open;
    fs_drv.close_cb = ImgFs_close;
    fs_drv.read_cb  = ImgFs_read;
    fs_drv.seek_cb  = ImgFs_seek;
    fs_drv.tell_cb  = ImgFs_tell;
    lv_fs_drv_register(&fs_drv);
}

const char * ImgFs_SlotPath(uint8_t slot)
{
    return (slot < IMG_SLOT_MAX) ? slot_paths[slot] : NULL;
}

static bool read_slot_header(uint8_t slot, uint32_t * v_out)
{
    if (slot >= IMG_SLOT_MAX) return false;

    uint8_t hdr[4] = {0};
    uint32_t addr = IMG_SLOT0_BASE + (uint32_t)slot * IMG_SLOT_STRIDE;
    if (!W25Q64_Read_Data(addr, hdr, 4)) return false;

    *v_out = hdr[0] | ((uint32_t)hdr[1] << 8) | ((uint32_t)hdr[2] << 16) | ((uint32_t)hdr[3] << 24);
    return true;
}

bool ImgFs_SlotHasValidImage(uint8_t slot)
{
    uint32_t v;
    if (!read_slot_header(slot, &v)) return false;

    //按lv_img_header_t(小端位域: cf在bit0-4, always_zero在bit5-7, reserved:2, w:11, h:11)解码
    uint32_t cf = v & 0x1F;
    uint32_t w  = (v >> 10) & 0x7FF;
    uint32_t h  = (v >> 21) & 0x7FF;

    //cf必须是TRUE_COLOR、尺寸非零、always_zero(bit5-7)为0
    return (cf == LV_IMG_CF_TRUE_COLOR) && w > 0 && h > 0 && ((v >> 5) & 0x7) == 0;
}

bool ImgFs_SlotSize(uint8_t slot, uint16_t * w, uint16_t * h)
{
    uint32_t v;
    if (!read_slot_header(slot, &v)) return false;
    *w = (v >> 10) & 0x7FF;
    *h = (v >> 21) & 0x7FF;
    return (*w > 0 && *h > 0);
}

void ImgFs_SelfTest(void)
{
    lv_fs_file_t f;
    lv_fs_res_t r = lv_fs_open(&f, "S:0.bin", LV_FS_MODE_RD);
    if (r != LV_FS_RES_OK) { Serial_Printf("IMGFS: open S:0.bin fail %d\r\n", r); return; }

    uint8_t hdr[4] = {0};
    uint32_t br = 0;
    r = lv_fs_read(&f, hdr, 4, &br);
    lv_fs_close(&f);
    //04 C0 03 23 = cf:TRUE_COLOR w:240 h:280
    Serial_Printf("IMGFS: read=%d br=%d hdr=%02X %02X %02X %02X (want 04 C0 03 23)\r\n",
                  r, br, hdr[0], hdr[1], hdr[2], hdr[3]);
}
