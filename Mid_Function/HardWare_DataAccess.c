//
// Created by a1191 on 2026/9/12.
//

#include "HardWare_DataAccess.h"

/**
 * @brief 基姆拉尔森公式计算周几
 * @param setyear 年
 * @param setmonth 月
 * @param setday 日
 * @param century 世纪
 * @return 1-7 对应周日到周六
 */
uint8_t HW_weekday_calculate(uint8_t setyear, uint8_t setmonth, uint8_t setday, uint8_t century)
{
    int w;
    if (setmonth == 1 || setmonth == 2)
    {setyear--, setmonth += 12;}
    w = setyear + setyear / 4 + century / 4  + 26*(setmonth + 1)/10 + setday - 1 - 2 * century;
    while(w<0)
        w+=7;
    w%=7;
    w=(w==0)?7:w;
    return w;
}

void HW_RTC_Set_Date(uint8_t year, uint8_t month, uint8_t date)
{
    #if HW_USE_RTC
        RTC_Set_Date(year, month, date);
    #endif
}


void HW_RTC_Set_Time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    #if HW_USE_RTC
        RTC_Set_Time(hours, minutes, seconds);
    #endif
}


void HW_RTC_Get_Date_Time(HW_DateTimeTypeDef* nowDateTime)
{
    //这里的#if,#else相当于手写多态
#if HW_USE_RTC
    if (nowDateTime!=NULL)
    {
        RTC_DateTypeDef nowDate;
        RTC_TimeTypeDef nowTime;
        //读时间寄存器时会把日历锁住，必须再读一次日期寄存器才会解锁
        HAL_RTC_GetTime(&hrtc,&nowTime,RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc,&nowDate,RTC_FORMAT_BIN);

        nowDateTime->Year=nowDate.Year;
        nowDateTime->Month=nowDate.Month;
        nowDateTime->Date=nowDate.Date;
        nowDateTime->WeekDay=weekday_calculate(nowDate.Year,nowDate.Month,nowDate.Date,20);
        nowDateTime->Hours=nowTime.Hours;
        nowDateTime->Minutes=nowTime.Minutes;
        nowDateTime->Seconds=nowTime.Seconds;
    }
    #else
    nowDateTime->Year = 26;
    nowDateTime->Month = 9;
    nowDateTime->Date = 13;
    nowDateTime->WeekDay = 1;
    nowDateTime->Hours = 22;
    nowDateTime->Minutes = 04;
    nowDateTime->Seconds = 55;
#endif

}


#if HW_USE_BLUETOOTH
void HW_Bluetooth_Set(bool on)
{
    Bluetooth_Set(on);
}

bool HW_Bluetooth_GetState(void)
{
    return Bluetooth_GetState();
}
#endif


#if HW_USE_LCD
void HW_LCD_Init(void)
{
    LCD_Init();
}

void HW_LCD_Set_Light(uint8_t dc)
{
    LCD_Set_Light(dc);
}

uint8_t HW_LCD_Get_Brightness(void)
{
    return LCD_Get_Backlight();
}

void HW_LCD_LightDown(void)
{
    LCD_LightDown();
}

void HW_LCD_Light_Open(void)
{
    LCD_Open_Light();
}

void HW_LCD_Close_Light(void)
{
    LCD_Close_Light();
}
#endif

#if HW_USE_DHT20
void HW_DHT20_Init(void)
{
    DHT20_Init();
}


void HW_DHT20_Read(float *Temperature,float *Humidity)
{
    DHT20_Read(Temperature,Humidity);
}

#endif



//这里实现了接口
HW_InterfaceTypeDef HWInterface={
#if HW_USE_RTC
    .RealTimeClock = {
        .CalculateWeekDay = HW_weekday_calculate,
        .SetDate = HW_RTC_Set_Date,
        .SetTime = HW_RTC_Set_Time,
        .GetDateTime = HW_RTC_Get_Date_Time,
    },
#endif

#if HW_USE_BLUETOOTH
    .Bluetooth = {
        .Set = HW_Bluetooth_Set,
        .GetState = HW_Bluetooth_GetState,
    },
#endif

#if HW_USE_LCD
    .LCD = {
        .Init = HW_LCD_Init,
        .Set_Light = HW_LCD_Set_Light,
        .Get_Brightness = HW_LCD_Get_Brightness,
        .LightDown = HW_LCD_LightDown,
        .Light_Close = HW_LCD_Close_Light,
        .Light_Open = HW_LCD_Light_Open,
    },
#endif

#if HW_USE_DHT20
    .DHT20 = {
        .Init = HW_DHT20_Init,
        .Read = HW_DHT20_Read,
    },
#endif

};