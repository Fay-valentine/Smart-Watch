/* Private includes -----------------------------------------------------------*/

// includes
// sys
#include "delay.h"
#include "sys.h"

// user
#include "user_TasksInit.h"

// bsp
#include "key.h"

// ui
#include "HardwareInitTask.h"

#include "HardWare_DataAccess.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "PageManager.h"
#include "Serial_command.h"
#include "tim.h"
#include "W25Q64_Task.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
extern osEventFlagsId_t BootEventHandle;
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief  hardwares init task
  * @param  argument: Not used
  * @retval None
  */
void StartHardwareInitTask(void *argument)
{
    while (1)
    {
        //delay
        delay_init();

        //PWM_LCD
        HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);

        //DHT20
        HWInterface.DHT20.Init();


        //MPU6050
        uint8_t num = 3;
        while (num && HWInterface.MPU6050.connection_error)
        {
            num--;
            HWInterface.MPU6050.connection_error = HWInterface.MPU6050.Init();
        }
        Serial_Printf("[MPU] init ret=%u (0=ok)\r\n", HWInterface.MPU6050.connection_error);
        osEventFlagsSet(BootEventHandle, BOOT_EVT_SENSOR_READY);


        //消息收件箱
        MessagePage_Inbox_Init(&message_page_inbox);//消息收件箱初始化

        //蓝牙(后开启蓝牙，防止消息收件箱未初始化，无法接收消息)
        HWInterface.Bluetooth.Set(true); //默认开启蓝牙

        //W25Q64
        HWInterface.W25Q64.Init();
        uint8_t failed_region_num = HWInterface.W25Q64.Log_Init_AllRegion();
        if (failed_region_num > 0)
        {
            Serial_Printf("[W25Q64] log init failed, failed region num=%u\r\n", failed_region_num);
        }
        //获取设置的时间和日期
        HWInterface.W25Q64.RestoreDateTime();

        //获取最近的五条消息
        HWInterface.W25Q64.Log_Foreach_Read(W25Q64_REGION_MSG_INBOX, W25Q64_Log_Foreach_Read_MsgInbox_cb,
                                            &read_msg_ctx);
        Serial_Printf("[W25Q64] inbox restored, got=%u msg(s)\r\n", read_msg_ctx.got_msg_num);
        osEventFlagsSet(BootEventHandle, BOOT_EVT_STORAGE_READY);


        vTaskSuspendAll();
        // LVGL and disp init(LVGL 和 触摸，显示初始化)
        lv_init();
        lv_port_disp_init();
        lv_port_indev_init();

        //页面栈
        Page_Init(); /* 页面栈接管首页加载，替代 SquareLine 生成的 ui_init()，避免双重加载/重复建屏 */

        xTaskResumeAll();

        osEventFlagsSet(BootEventHandle, BOOT_EVT_UI_READY);

        vTaskDelete(NULL);
    }
}
