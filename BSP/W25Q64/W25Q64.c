//
// Created by a1191 on 2026/10/4.
//

#include "W25Q64.h"

#include <string.h>
#include <sys/types.h>

#include "main.h"
#include "gpio.h"
#include "HardWare_DataAccess.h"
#include "SerialTask.h"
#include "Serial_command.h"
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
void  W25Q64_Init(void)
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

//用于日志式追加，用于每种类型的region
typedef struct
{
    uint16_t region;
    uint32_t sector_addr;
    uint16_t slot_size;    // 必须整除256：8/16/32/64/128/256，槽才不跨页
    uint16_t slot_count;   // =4096/slot_size
    uint16_t slot_idx;    // 写指针，上电扫描重建
    uint16_t last_valid_idx;   // 最后一条有效记录的槽号；0xFFFF=一条都没有
    uint16_t seq;//1,2,3...
} W25Q64_Log_t;

static W25Q64_Log_t logs[] = {
    { W25Q64_REGION_DATETIME,  0x001000, 16, 4096/16, 0, 0xFFFF, 1 },
    { W25Q64_REGION_MSG_INBOX, 0x002000, 64,  4096/64, 0, 0xFFFF, 1 },
};

#define W25Q64_LOG_NUM (sizeof(logs)/sizeof(logs[0]))

/**
 * @brief
 * @param slot
 * @param hdr
 */
static void frame_hdr_decode(const uint8_t *slot, W25Q64_Log_Frame_Hdr_t *hdr)
{
    hdr->magic  = (uint16_t)(slot[0] | ((uint16_t)slot[1] << 8));
    hdr->seq    = (uint16_t)(slot[2] | ((uint16_t)slot[3] << 8));
    hdr->region = (uint16_t)(slot[4] | ((uint16_t)slot[5] << 8));
    hdr->len    = slot[6];
    hdr->checksum     = slot[7];
}

/**
 * @brief 包装slot的帧头，方便查看和操作
 */
static void frame_hdr_encode(const W25Q64_Log_Frame_Hdr_t *hdr, uint8_t *slot)
{
    slot[0] = (uint8_t)hdr->magic;   slot[1] = (uint8_t)(hdr->magic  >> 8);
    slot[2] = (uint8_t)hdr->seq;     slot[3] = (uint8_t)(hdr->seq    >> 8);
    slot[4] = (uint8_t)hdr->region;  slot[5] = (uint8_t)(hdr->region >> 8);
    slot[6] = hdr->len;
    slot[7] = hdr->checksum;
}


/**
 * @brief 根据region，查表得到对应的log结构体
 * @param region 接收到的region
 * @return region对应的log结构体
 */
static W25Q64_Log_t *which_log(uint16_t region)
{
    for (uint16_t i = 0; i < W25Q64_LOG_NUM; i++)
    {
        if (logs[i].region == region)
        {
            return &logs[i];
        }
    }
    return NULL;
}

/**
 * @brief 计算校验和，包括帧头和数据负载
 * @param slot
 * @param payload_len
 */
static uint8_t slot_compute_checksum(const uint8_t *slot, uint16_t payload_len)
{
    uint8_t sum = 0;
    for (uint8_t i = 0; i < W25Q64_LOG_FRAME_HDR_SIZE - 1; i++)//帧头，不包括校验位本身
    {
        sum += slot[i];
    }
    for (uint16_t i = 0; i < payload_len; i++)//数据负载
    {
        sum += slot[W25Q64_LOG_FRAME_HDR_SIZE + i];
    }
        return sum;
}

/**
 * @brief 检验slot是否合法，包括magic,region，数据长度，校验和
 * @param slot
 * @param L
 * @return 0：不合法
 */
static uint8_t slot_is_valid(const uint8_t *slot, const W25Q64_Log_t *L)
{
    W25Q64_Log_Frame_Hdr_t hdr;
    frame_hdr_decode(slot, &hdr);

    //magic,region,不对，或数据过长，无效
    if (hdr.magic!=W25Q64_LOG_SLOT_MAGIC || hdr.region!=L->region || hdr.len+W25Q64_LOG_FRAME_HDR_SIZE>L->slot_size)
    {
        return 0;
    }

    return slot_compute_checksum(slot,hdr.len)==hdr.checksum;
}

/**
 * @brief 从slot的末尾开始扫描，检查slot是否为空（即是否全为0xFF,未被写过）
 * @return 1：slot为空  0：slot已经被写过
 */
static uint8_t slot_is_empty(const uint8_t *slot, uint16_t slot_size)
{
    while (slot_size--)
    {
        if (slot[slot_size] != 0xFF)
        {
            return 0;
        }
    }
        return 1;
}

/**
 * @brief 扫描扇区，找到第一个空的slot,需判断扇区是否被写过，是否包含有效数据；若包含有效数据，则把写指针定位到最新有效数据的后面
 */
static uint8_t Log_Init_oneRegion(W25Q64_Log_t* L)
{
    uint8_t sector_written=0;
    uint8_t contain_valid_data=0;
    uint8_t slot[L->slot_size];

    //1.遍历扇区的每一个槽,找到最新的有效数据位置（要防止两条有效数据间有无效数据，导致写指针停在中间，所以要先遍历整个扇区）
    for (int i=0;i<L->slot_count;++i)
    {
        if (!W25Q64_Read_Data(L->sector_addr+i*L->slot_size,slot,L->slot_size ))//读取slot
        {
            return 0;
        }

        W25Q64_Log_Frame_Hdr_t hdr;
        frame_hdr_decode(slot, &hdr);//包装帧头，以便操作

        if (!slot_is_empty(slot,L->slot_size))//扇区被写过
        {
            sector_written=1;
        }
        if (slot_is_valid(slot,L))//刷新有效数据的位置并记录
        {
            L->last_valid_idx=i;//记录位置
            L->seq=hdr.seq;//记录序号
            contain_valid_data=1;
        }
    }

    //2.从最新的有效数据后面寻找空槽
    if (contain_valid_data)
    {
        L->slot_idx=L->slot_count;//防止下面循环没有找到空槽，L->slot_idx未被更新
        for (int i=L->last_valid_idx+1;i<L->slot_count;++i)
        {
            if (!W25Q64_Read_Data(L->sector_addr+i*L->slot_size,slot,L->slot_size ))
            {
                return 0;
            }
            if (slot_is_empty(slot,L->slot_size))
            {
                L->slot_idx=i;
                break;
            }
        }
    }
    else//无 有效信息
    {
        if (sector_written)//擦除旧扇区，再定位写指针
        {
            Serial_Printf("[W25Q64] region=%u legacy data, sector erased once\r\n", L->region);
            if (!W25Q64_Sector_Erase(L->sector_addr))
            {
                return 0;
            }

        }
        L->slot_idx=0;
        L->seq=1;
    }
    return 1;
}

/**
 * @brief 初始化所有日志分区
 * @return 初始化失败的分区数量
 */
uint8_t W25Q64_Log_Init_AllRegion(void)
{
    uint8_t failed_region_num=0;
    for (int i=0;i<W25Q64_LOG_NUM;++i)
    {
        if (!Log_Init_oneRegion(&logs[i]))
        {
            failed_region_num++;
        }
    }
    return failed_region_num;
}

/**
 * @brief 向指定region追加slot
 */
uint8_t W25Q64_Log_Append(uint16_t region,const uint8_t* payload,uint16_t payload_len)
{
    W25Q64_Log_t* L=which_log(region);

    if (L==NULL || payload_len==0 || payload_len+W25Q64_LOG_FRAME_HDR_SIZE>L->slot_size)
    {
        Serial_Printf("[W25Q64] append reject(invalid params): region=%u len=%u\r\n", region, payload_len);
        return 0;
    }

    uint8_t slot[L->slot_size];

    //找到空槽
    while (1)
    {
        //先检验扇区是否写满了
        if (L->slot_idx>=L->slot_count)
        {
            Serial_Printf("[W25Q64] region=%u log full, sector erased\r\n", L->region);
            //擦除旧扇区，再定位写指针
            if (!W25Q64_Sector_Erase(L->sector_addr))
            {
                return 0;
            }
            L->slot_idx=0;
            L->last_valid_idx=0xFFFF;
            L->seq=1;
            break;
        }

        if (!W25Q64_Read_Data(L->sector_addr+L->slot_idx*L->slot_size,slot,L->slot_size ))
        {
            return 0;
        }

        if (slot_is_empty(slot,L->slot_size))
        {
            break;
        }
        else//有无效数据，跳过
        {
            L->slot_idx++;
        }
    }

    //封装帧头
    W25Q64_Log_Frame_Hdr_t hdr=
    {
        .magic = W25Q64_LOG_SLOT_MAGIC,
        .region = region,
        .len = payload_len,
        .seq = L->seq++,
        .checksum = 0,
    };

    //在帧头后添加数据负载
    for (uint16_t i = 0; i < payload_len; i++)
    {
        slot[W25Q64_LOG_FRAME_HDR_SIZE + i] = payload[i];
    }

    //空余数据位填充0x00
    for (uint16_t i = W25Q64_LOG_FRAME_HDR_SIZE + payload_len; i < L->slot_size; i++)
    {
        slot[i] = 0x00;
    }

    frame_hdr_encode(&hdr,slot);//将帧头写入slot
    //计算校验和
    slot[W25Q64_LOG_FRAME_HDR_SIZE-1]=slot_compute_checksum(slot,payload_len);



    //写入W25Q64
    if (!W25Q64_Page_Program(L->sector_addr+L->slot_idx*L->slot_size,slot,L->slot_size))
    {
        return 0;
    }

    //更新有效数据的位置
    L->last_valid_idx=L->slot_idx;
    L->slot_idx++;

    Serial_Printf("[W25Q64] append ok: region=%u slot=%u seq=%u\r\n",region, L->last_valid_idx, L->seq - 1);

    return 1;
}

/**
 * @brief 读取日志分区最后一个有效槽的数据
 * @param region 分区编号
 * @param payload 数据缓冲区
 * @param payload_max_len 数据缓冲区最大长度
 * @param read_len 实际读取的数据长度
 * @return 读取成功返回1，失败返回0
 */
uint8_t W25Q64_Log_Read_LastSlot(uint16_t region,uint8_t* payload,uint16_t payload_max_len,uint16_t* read_len)
{
    W25Q64_Log_t* L=which_log(region);
    if (L==NULL || L->last_valid_idx==0xFFFF)//无有效数据
    {
        if (read_len!=NULL)
        {
            *read_len=0;
        }
        return 0;
    }

    uint8_t slot[L->slot_size];
    if (!W25Q64_Read_Data(L->sector_addr+L->last_valid_idx*L->slot_size,slot,L->slot_size))
    {
        return 0;
    }

    //防止运行过程中，最后一条数据，被写入了无效数据
    if (!slot_is_valid(slot,L))
    {
        if (read_len!=NULL)
        {
            *read_len=0;
        }
        return 0;
    }

    //解析帧头
    W25Q64_Log_Frame_Hdr_t hdr;
    frame_hdr_decode(slot,&hdr);

    //防止数据长度 超过 传入数组的最大长度
    uint16_t payload_len=hdr.len;
    if (payload_len>payload_max_len)
    {
        payload_len=payload_max_len;
    }
    if (read_len!=NULL)
    {
        *read_len=payload_len;//实际读取的数据长度
    }

    //将数据拷贝至传入数组
    for (int i=0;i<payload_len;++i)
    {
        payload[i]=slot[W25Q64_LOG_FRAME_HDR_SIZE+i];
    }

    return 1;
}

/**
 * @brief 从region末尾到开头，遍历slot
 * @param arg 回调函数的参数，决定遍历的方式
 * @return 遍历的slot数量
 */
uint8_t W25Q64_Log_Foreach_Read(uint16_t region,W25Q64_Log_Foreach_Read_cb cb,void* arg)
{
    W25Q64_Log_t* L=which_log(region);
    if (L==NULL || L->last_valid_idx==0xFFFF)//无有效数据
    {
        return 0;
    }

    uint8_t slot[L->slot_size];
    uint16_t foreach_slot_num=0;

    for (int i=L->last_valid_idx;i>=0;i--)
    {
        if (!W25Q64_Read_Data(L->sector_addr+i*L->slot_size,slot,L->slot_size))
        {
            return 0;
        }
        //slot无效则跳过
        if (!slot_is_valid(slot,L))
        {
            continue;
        }

        //解析帧头，用于下面获取 数据长度
        W25Q64_Log_Frame_Hdr_t hdr;
        frame_hdr_decode(slot,&hdr);

        //调用回调函数，由 回调函数 控制遍历是否结束
        if (cb(slot+W25Q64_LOG_FRAME_HDR_SIZE,hdr.len,arg)==0)
        {
            break;
        }
        foreach_slot_num++;//统计遍历的slot数量
    }
    return foreach_slot_num;
}

/**
 *  @brief 擦除对应region的扇区
 */
uint8_t W25Q64_Log_Reset_Region(uint16_t region)
{
    W25Q64_Log_t *L = which_log(region);
    if (L == NULL)
    {
        return 0;
    }
    if (!W25Q64_Sector_Erase(L->sector_addr))
    {
        return 0;
    }
    L->slot_idx = 0;
    L->last_valid_idx = 0xFFFF;
    return 1;
}

/**
 * @brief 将当前RTC时间日期发送到 W25Q64消息队列，让 W25Q64任务 保存到 日期时间分区
 */
void W25Q64_Saved_Current_DateTime(void)
{
    DateTimeData_t date_time_data;
    HW_DateTimeTypeDef now;
    StorageReq_t request;

    //获取时间
    HWInterface.RealTimeClock.GetDateTime(&now);

    date_time_data.year=now.Year;
    date_time_data.month=now.Month;
    date_time_data.day=now.Date;
    date_time_data.hour=now.Hours;
    date_time_data.minute=now.Minutes;
    date_time_data.second=now.Seconds;

    request.region=W25Q64_REGION_DATETIME;
    request.len=sizeof(DateTimeData_t);

    //将快照拷贝至存储请求结构体，以用于 消息队列
    memcpy(request.data,&date_time_data,sizeof(DateTimeData_t));

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
    DateTimeData_t date_time_data;
    uint16_t read_len=0;

    //读取datetime分区的最后一条slot
    if (W25Q64_Log_Read_LastSlot(W25Q64_REGION_DATETIME,(uint8_t*)&date_time_data,sizeof(DateTimeData_t),&read_len) &&
        read_len==sizeof(DateTimeData_t) &&
        date_time_data.month >= 1 && date_time_data.month <= 12 && date_time_data.day >= 1 && date_time_data.day <= 31 &&
        date_time_data.hour <= 23 && date_time_data.minute <= 59 && date_time_data.second <= 59)
    {
        //修改RTC时间日期
        HWInterface.RealTimeClock.SetDate(date_time_data.year,date_time_data.month,date_time_data.day);
        HWInterface.RealTimeClock.SetTime(date_time_data.hour,date_time_data.minute,date_time_data.second);

        Serial_Printf("[W25Q64] time restored: 20%02u-%02u-%02u %02u:%02u:%02u\r\n",
                      date_time_data.year, date_time_data.month, date_time_data.day,
                      date_time_data.hour, date_time_data.minute, date_time_data.second);
    }
    else
    {
        Serial_Printf("[W25Q64] no valid time in flash, use default RTC\r\n");
    }
}