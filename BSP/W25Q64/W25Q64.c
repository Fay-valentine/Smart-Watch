//
// Created by a1191 on 2026/10/4.
//

#include "W25Q64.h"

#include <string.h>

#include "main.h"
#include "gpio.h"
#include "HardWare_DataAccess.h"
#include "SerialTask.h"
#include "spi.h"
#include "W25Q64_Instruction.h"

extern osMutexId_t W25Q64_RecursiveMutexHandle;
extern osMessageQueueId_t W25Q64QueueHandle;

/* 1字节≈10µs @0.78MHz，len字节≈len*0.01ms，再给10ms固定余量 */
#define SPI_XFER_TIMEOUT(len)   (10u + (len) / 100u)       // 单次SPI收发的最大等待时间，ms
#define FLASH_BUSY_TIMEOUT 600    /* 等Flash空闲，ms，盖住扇区擦除max~400ms */

/**
 * @brief 拉高片选电平，不选中W25Q64
 */
static void W25Q64_CS_High(void)
{
    HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port,W25Q64_CS_Pin,GPIO_PIN_SET);
}

/**
 * @brief 拉低片选电平，选中W25Q64
 */
static void W25Q64_CS_Low(void)
{
    HAL_GPIO_WritePin(W25Q64_CS_GPIO_Port,W25Q64_CS_Pin,GPIO_PIN_RESET);
}

/**
 * @brief SPI2发送数据
 * @param data
 * @param len
 * @return 1: 成功  0: 失败
 */
static uint8_t SPI2_Send(const uint8_t *data, uint16_t len)
{
    return HAL_SPI_Transmit(&hspi2, (uint8_t *)data, len, SPI_XFER_TIMEOUT(len)) == HAL_OK;
}

/**
 * @brief SPI2接收数据
 * @param data
 * @param len
 * @return 1: 成功  0: 失败
 */
static uint8_t SPI2_Recv(uint8_t *data, uint16_t len)
{
    return HAL_SPI_Receive(&hspi2, data, len, SPI_XFER_TIMEOUT(len)) == HAL_OK;
}

static uint8_t W25Q64_ReadStatus(void)
{
    uint8_t cmd = W25Q64_READ_STATUS_REGISTER_1;
    uint8_t status = 0xFF;
    W25Q64_CS_Low();
    if (SPI2_Send(&cmd, 1))
    {
        SPI2_Recv(&status, 1);
    }
    W25Q64_CS_High();
    return status;
}


/**
 * @brief 默认不选中W25Q64
 */
void W25Q64_Init(void)
{
    W25Q64_CS_High();
}

/**
 * @brief 读取W25Q64的Manufacturer ID和Device ID
 * @param MID Manufacturer ID
 * @param DID Device ID
 * @note 未读取到，则两个ID均为0
 */
void W25Q64_ReadID(uint8_t* MID,uint16_t* DID)
{
    uint8_t cmd=W25Q64_JEDEC_ID;
    uint8_t receive[3]={0};

    //获取互斥锁
    osMutexAcquire(W25Q64_RecursiveMutexHandle,osWaitForever);

    //选中芯片
    W25Q64_CS_Low();

    //发送命令  接收数据
    if (SPI2_Send(&cmd,1))
    {
        SPI2_Recv(receive,3);
    }

    //解析数据
    *MID=receive[0];
    *DID=(receive[1]<<8)|receive[2];

    //取消选中
    W25Q64_CS_High();

    //释放互斥锁
    osMutexRelease(W25Q64_RecursiveMutexHandle);
}

uint8_t W25Q64_WriteEnable(void)
{
    uint8_t cmd=W25Q64_WRITE_ENABLE;
    uint8_t sent=0;//cmd是否被发送 0:未发送 1:已发送
    osMutexAcquire(W25Q64_RecursiveMutexHandle,osWaitForever);
    //选中芯片
    W25Q64_CS_Low();
    //发送命令
    sent = SPI2_Send(&cmd,1);
    //取消选中
    W25Q64_CS_High();
    osMutexRelease(W25Q64_RecursiveMutexHandle);
    return sent;
}

/**
 * @return 0: 忙 1: 空闲
 */
uint8_t W25Q64_Wait_Busy(void)
{
    osMutexAcquire(W25Q64_RecursiveMutexHandle, osWaitForever);
    uint8_t ready = 0;
    for (uint32_t waited = 0; waited < FLASH_BUSY_TIMEOUT; waited++)//600ms
    {
        if ((W25Q64_ReadStatus() & 0x01) == 0)
        {
            ready = 1;
            break;
        }
        osDelay(1);             /* 让出CPU，擦除的45ms里系统照常跑别的任务 */
    }
    osMutexRelease(W25Q64_RecursiveMutexHandle);
    return ready;
}

/**
  * @brief W25Q64页编程
  * @param address 页编程的起始地址，范围：0x000000~0x7FFFFF
  * @param dataArray	用于写入数据的数组
  * @param count 要写入数据的数量，范围：0~256
  * @return 1: 成功  0: 失败
  * @note 写入的地址范围不能跨页
  */
uint8_t W25Q64_Page_Program(uint32_t address, uint8_t* dataArray, uint16_t count)
{
    uint8_t cmd[4]={W25Q64_PAGE_PROGRAM,(uint8_t)(address>>16)&0xFF,
                    (uint8_t)(address>>8)&0xFF,(uint8_t)address&0xFF};

    uint8_t success=0;//是否页编程成功 0:失败 1:成功


    osMutexAcquire(W25Q64_RecursiveMutexHandle,osWaitForever);

    if (W25Q64_WriteEnable())
    {
        W25Q64_CS_Low();

        //cmd是否被发送 0:未发送 1:已发送
        uint8_t sent = SPI2_Send(cmd,4) && SPI2_Send(dataArray,count);

        W25Q64_CS_High();

        success = sent && W25Q64_Wait_Busy();//等到页编程完成后，才能释放互斥锁
    }

    osMutexRelease(W25Q64_RecursiveMutexHandle);

    return success;
}

/**
  * @brief  W25Q64扇区擦除（4KB）
  * @param address 指定扇区的地址，范围：0x000000~0x7FFFFF
  * @return 1: 成功  0: 失败
  */
uint8_t W25Q64_Sector_Erase(uint32_t address)
{
    uint8_t cmd[4]={W25Q64_SECTOR_ERASE_4KB,(uint8_t)(address>>16)&0xFF,
                    (uint8_t)(address>>8)&0xFF,(uint8_t)address&0xFF};

    uint8_t success=0;//是否擦除成功 0:失败 1:成功

    osMutexAcquire(W25Q64_RecursiveMutexHandle,osWaitForever);

    if (W25Q64_WriteEnable())
    {
        W25Q64_CS_Low();

        uint8_t sent = SPI2_Send(cmd,4);

        W25Q64_CS_High();

        success = sent && W25Q64_Wait_Busy();//等到扇区擦除完成后，才能释放互斥锁
    }

    osMutexRelease(W25Q64_RecursiveMutexHandle);

    return success;
}

/**
  * @brief W25Q64读取数据
  * @param address 读取数据的起始地址，范围：0x000000~0x7FFFFF
  * @param dataArray 用于接收读取数据的数组，通过输出参数返回
  * @param count 要读取数据的数量，范围：0~0x800000
  * @return 1: 成功  0: 失败
  */
uint8_t W25Q64_Read_Data(uint32_t address, uint8_t* dataArray, uint32_t count)
{
    uint8_t cmd[4]={W25Q64_READ_DATA,(uint8_t)(address>>16)&0xFF,
                    (uint8_t)(address>>8)&0xFF,(uint8_t)address&0xFF};

    uint8_t sent=0;//cmd是否被发送 0:未发送 1:已发送

    osMutexAcquire(W25Q64_RecursiveMutexHandle,osWaitForever);

    W25Q64_CS_Low();

    sent = SPI2_Send(cmd,4) && SPI2_Recv(dataArray,count);

    W25Q64_CS_High();

    osMutexRelease(W25Q64_RecursiveMutexHandle);

    return sent;
}

/**
 * @brief 将当前RTC时间日期发送到 W25Q64消息队列，让 W25Q64任务 保存到 日期时间分区
 */
void W25Q64_Saved_Current_DateTime(void)
{
    DateTimeStore_t snap;
    HW_DateTimeTypeDef now;
    StorageReq_t request;

    //获取时间
    HWInterface.RealTimeClock.GetDateTime(&now);

    //保存快照信息
    snap.magic    = W25Q64_DATETIME_MAGIC;
    snap.year     = now.Year;
    snap.month    = now.Month;
    snap.day      = now.Date;
    snap.hour     = now.Hours;
    snap.minute   = now.Minutes;
    snap.second   = now.Seconds;
    snap.checksum=(uint8_t)(snap.magic>>8)+(uint8_t)snap.magic+
                   snap.year+snap.month+snap.day+
                   snap.hour+snap.minute+snap.second;

    request.region=W25Q64_REGION_DATETIME;
    request.len=sizeof(DateTimeStore_t);
    //将快照拷贝至存储请求结构体，以用于 消息队列
    memcpy(request.data,&snap,sizeof(snap));

    if (W25Q64QueueHandle!=NULL &&
        osMessageQueuePut(W25Q64QueueHandle,&request,0,0)!=osOK)
    {
        Serial_Printf("Fail to save current date and time to W25Q64");
    }
}

/**
 * @brief 从W25Q64的 日期时间分区 读取日期时间，并修改RTC
 */
void W25Q64_RestoreDateTime(void)
{
    DateTimeStore_t store;

    //读取数据
    if (!W25Q64_Read_Data(W25Q64_REGION_DATETIME_ADDR, (uint8_t *)&store, sizeof(store)))
    {
        Serial_Printf("[W25Q64] restore read fail, use default RTC\r\n");
        return;
    }

    //计算校验和
    uint8_t check_sum = 0;
    check_sum += (uint8_t)(store.magic >> 8) + (uint8_t)store.magic+
            store.year + store.month + store.day+
            store.hour + store.minute + store.second;

    //校验数据
    if (store.magic == W25Q64_DATETIME_MAGIC &&
        store.checksum == check_sum &&
        store.month >= 1 && store.month <= 12 && store.day >= 1 &&
        store.day <= 31 &&store.hour <= 23 && store.minute <= 59)
    {
        HWInterface.RealTimeClock.SetDate(store.year, store.month, store.day);  // 内部自动算星期
        HWInterface.RealTimeClock.SetTime(store.hour, store.minute, store.second);
        Serial_Printf("[W25Q64] time restored: 20%02u-%02u-%02u %02u:%02u:%02u\r\n",
                      store.year, store.month, store.day, store.hour, store.minute, store.second);
    }
    else
    {
        Serial_Printf("[W25Q64] no valid time in flash (magic=%04X), use default RTC\r\n", store.magic);
    }
}

