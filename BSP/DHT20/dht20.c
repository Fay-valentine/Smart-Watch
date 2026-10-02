#include "dht20.h"
#include "iic_hal.h"

#define DHT20_ADDRESS   0x38


static iic_bus_t aht20_bus={
    .IIC_SCL_PORT = DHT20_SCL_GPIO_Port,
    .IIC_SCL_PIN = DHT20_SCL_Pin,
    .IIC_SDA_PORT = DHT20_SDA_GPIO_Port,
    .IIC_SDA_PIN = DHT20_SDA_Pin,
};

/**
 * @brief DHT20专用写事务：START + 写地址 + 一串裸字节 + STOP
 *        (不能用IIC_Write_*：它们强制先发一个"寄存器字节")
 */
static uint8_t DHT20_WriteBytes(const uint8_t *data, uint8_t len)
{
    IICStart(&aht20_bus);
    IICSendByte(&aht20_bus, DHT20_ADDRESS << 1);
    if (IICWaitAck(&aht20_bus)) { IICStop(&aht20_bus); return 1; }
    for (uint8_t i = 0; i < len; i++)
    {
        IICSendByte(&aht20_bus, data[i]);
        if (IICWaitAck(&aht20_bus)) { IICStop(&aht20_bus); return 1; }
    }
    IICStop(&aht20_bus);
    return 0;
}

/**
 * @brief DHT20专用读事务：START + 读地址 + len字节(末字节NACK) + STOP
 */
static uint8_t DHT20_ReadBytes(uint8_t *buf, uint8_t len)
{
    IICStart(&aht20_bus);
    IICSendByte(&aht20_bus, (DHT20_ADDRESS << 1) | 1);
    if (IICWaitAck(&aht20_bus)) { IICStop(&aht20_bus); return 1; }
    for (uint8_t i = 0; i < len; i++)
    {
        buf[i] = IICReceiveByte(&aht20_bus);
        if (i < (len - 1)) IICSendAck(&aht20_bus);
        else               IICSendNotAck(&aht20_bus);
    }
    IICStop(&aht20_bus);
    return 0;
}


void DHT20_Init(void)
{
    uint8_t status = 0;
    osDelay(100);//上电后等待100ms
    DHT20_ReadBytes(&status, 1);
    if ((status & 0x18) != 0x18)//如果状态字和0x18相与后不等于0x18，初始化0x1B、0x1C、0x1E寄存器
    {
        uint8_t init_cmd[3] = {0x1B, 0x1C, 0x1E};
        DHT20_WriteBytes(init_cmd, 3);
    }
}


/**
 *
 * @param Temperature 摄氏度
 * @param Humidity 湿度,百分比形式
 */
void DHT20_Read(float *Temperature,float *Humidity)
{
    osDelay(10);//等待10ms后才发送测量命令
    uint8_t sendBuffer[3]={0xAC,0x33,0x00};//测量命令
    uint8_t read_Data_Buffer[6]={0};//读取数据缓存区

    if (DHT20_WriteBytes(sendBuffer,3))//发送测量命令,未成功则return
    {
        return;
    }
    osDelay(80);//等待80ms待测量完成


    for (int i=0;i<10;++i)//超时等待
    {
        if (DHT20_ReadBytes(read_Data_Buffer,6)==1)//读取失败也需要return
        {
            return;
        }

        //如果读取状态字Bit[7]为0，表示测量完成，然后可以连续读取六个字节；否则继续等待
        if((read_Data_Buffer[0]&0x80)==0x00)
        {
            uint32_t data=0;
            data=((uint32_t)read_Data_Buffer[3]>>4)+((uint32_t)read_Data_Buffer[2]<<4)+((uint32_t)read_Data_Buffer[1]<<12);
            *Humidity=data*100.f/(1<<20);

            data=((uint32_t)read_Data_Buffer[5])+((uint32_t)read_Data_Buffer[4]<<8)+(((uint32_t)read_Data_Buffer[3]&0x0F)<<16);
            *Temperature=data*200.0f/(1<<20)-50;
            break;
        }
        osDelay(10);
    }
}

// void DHT20_Init(void)
// {
//     uint8_t status = 0;
//     osDelay(100);//上电后等待100ms
//     HAL_I2C_Master_Transmit_DMA(&hi2c1,DHT20_ADDRESS<<1,&status,1);
//     if ((status & 0x18) != 0x18)//如果状态字和0x18相与后不等于0x18，初始化0x1B、0x1C、0x1E寄存器
//     {
//         uint8_t init_cmd[3] = {0x1B, 0x1C, 0x1E};
//         HAL_I2C_Master_Transmit_DMA(&hi2c1,DHT20_ADDRESS<<1,init_cmd,3);
//     }
// }
//
//
// /**
//  *
//  * @param Temperature 摄氏度
//  * @param Humidity 湿度,百分比形式
//  */
// void DHT20_Read(float *Temperature,float *Humidity)
// {
//     osDelay(10);//等待10ms后才发送测量命令
//     uint8_t sendBuffer[3]={0xAC,0x33,0x00};//测量命令
//     uint8_t read_Data_Buffer[6]={0};//读取数据缓存区
//
//     HAL_I2C_Master_Transmit_DMA(&hi2c1,DHT20_ADDRESS<<1,sendBuffer,3);
//     osDelay(80);//等待80ms待测量完成
//
//
//     for (int i=0;i<10;++i)//超时等待
//     {
//         HAL_I2C_Master_Receive_DMA(&hi2c1,DHT20_ADDRESS<<1,read_Data_Buffer,6);
//
//         //如果读取状态字Bit[7]为0，表示测量完成，然后可以连续读取六个字节；否则继续等待
//         if((read_Data_Buffer[0]&0x80)==0x00)
//         {
//             uint32_t data=0;
//             data=((uint32_t)read_Data_Buffer[3]>>4)+((uint32_t)read_Data_Buffer[2]<<4)+((uint32_t)read_Data_Buffer[1]<<12);
//             *Humidity=data*100.f/(1<<20);
//
//             data=((uint32_t)read_Data_Buffer[5])+((uint32_t)read_Data_Buffer[4]<<8)+(((uint32_t)read_Data_Buffer[3]&0x0F)<<16);
//             *Temperature=data*200.0f/(1<<20)-50;
//             break;
//         }
//         osDelay(10);
//     }
// }