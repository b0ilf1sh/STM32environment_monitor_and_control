#ifndef _DHT11_H_
#define	_DHT11_H_

#include "hardware.h"

#define DHT11_DATA_NUM    5    //´æ´¢ÊýÁ¿

void DHT11_OFF(void);
uint8_t DHT11_GetData(uint8_t *humidity, uint8_t *temperature);
void DHT11_Init(void);
void DHT11_Run(uint8_t *humidity, uint8_t *temperature);

#endif
