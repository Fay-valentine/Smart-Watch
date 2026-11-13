#ifndef __USER_HARDWAREINITTASK_H__
#define __USER_HARDWAREINITTASK_H__

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

void StartHardwareInitTask(void *argument);

#define BOOT_EVT_SENSOR_READY    (1U << 0)   /* DHT20 / MPU6050 初始化完成 */
#define BOOT_EVT_STORAGE_READY   (1U << 1)   /* W25Q64 初始化 + 时间扇区/收件箱扇区恢复完成 */
#define BOOT_EVT_UI_READY        (1U << 2)   /* LVGL / 显示 / 输入 / 页面栈初始化完成 */


#endif

