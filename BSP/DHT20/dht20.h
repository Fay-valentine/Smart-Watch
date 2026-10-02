#ifndef _AHT20_H
#define _AHT20_H

#include "i2c.h"

void DHT20_Init();
void DHT20_Read(float *Temperature,float *Humidity);


#endif 