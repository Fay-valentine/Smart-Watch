#include "key.h"
#include "delay.h"

/**
 * @brief 按键扫描
 * @param mode 0：单击模式  1：连按模式
 * @return 0：未按下  1：按键1
 */
uint8_t KeyScan(uint8_t mode)
{
	static uint8_t key_up = 1;
	uint8_t key_value=0;
	if(mode) key_up = 1;
	if( key_up && (!KEY1))
	{
		delay_ms(3);//ensure the key is down
		if(!KEY1) key_value = 1;
		if(key_value) key_up = 0;
	}
	else
	{
		delay_ms(3);//ensure the key is up
		if(KEY1)
			key_up = 1;
	}
	return key_value;
}

