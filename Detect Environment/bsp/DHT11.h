#ifndef _DHT11_H_
#define	_DHT11_H_

#include "hardware.h"

void DHT11_OFF(void);
uint8_t DHT11_GetData(uint8_t *humidity, uint8_t *temperature);

#endif
