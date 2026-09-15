//
// Created by a1191 on 2026/9/12.
//

#ifndef INC_10_WATCH_HARDWARE_DATAACCESS_H
#define INC_10_WATCH_HARDWARE_DATAACCESS_H

#define HW_USE_HARDWARE     1

#if HW_USE_HARDWARE

    #define HW_USE_RTC  1

#endif

#if HW_USE_RTC
    #include "rtc.h"
#endif


/**
  * @brief  HW RTC DateTime structure definition
  */
typedef struct
{
    uint8_t WeekDay;    /*!< Specifies the RTC Date WeekDay.
                            This parameter can be a value of @ref RTC_WeekDay_Definitions */

    uint8_t Month;      /*!< Specifies the RTC Date Month (in BCD format).
                            This parameter can be a value of @ref RTC_Month_Date_Definitions */

    uint8_t Date;       /*!< Specifies the RTC Date.
                            This parameter must be a number between Min_Data = 1 and Max_Data = 31 */

    uint8_t Year;       /*!< Specifies the RTC Date Year.
                            This parameter must be a number between Min_Data = 0 and Max_Data = 99 */

    uint8_t Hours;      /*!< Specifies the RTC Time Hour.
                            This parameter must be a number between Min_Data = 0 and Max_Data = 12 if the RTC_HourFormat_12 is selected
                            This parameter must be a number between Min_Data = 0 and Max_Data = 23 if the RTC_HourFormat_24 is selected */

    uint8_t Minutes;    /*!< Specifies the RTC Time Minutes.
                            This parameter must be a number between Min_Data = 0 and Max_Data = 59 */

    uint8_t Seconds;    /*!< Specifies the RTC Time Seconds.
                            This parameter must be a number between Min_Data = 0 and Max_Data = 59 */

} HW_DateTimeTypeDef;


typedef struct
{
    uint8_t(*CalculateWeekDay)(uint8_t setyear, uint8_t setmonth, uint8_t setday, uint8_t century);
    void(*SetDate)(uint8_t year, uint8_t month, uint8_t date);
    void(*SetTime)(uint8_t hours, uint8_t minutes, uint8_t seconds);
    void(*GetDateTime)(HW_DateTimeTypeDef* nowDateTime);
}HW_RTC_InterfaceTypeDef;

typedef struct
{
    HW_RTC_InterfaceTypeDef RealTimeClock;
}HW_InterfaceTypeDef;

extern HW_InterfaceTypeDef HWInterface;//硬件接口

#endif //INC_10_WATCH_HARDWARE_DATAACCESS_H