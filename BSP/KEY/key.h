#ifndef __KEY_H__
#define __KEY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

//KEY1
#define KEY1_VALUE  1
#define KEY1 HAL_GPIO_ReadPin(Key1_GPIO_Port,Key1_Pin)

uint8_t KeyScan(uint8_t mode);




#ifdef __cplusplus
}
#endif
#endif

