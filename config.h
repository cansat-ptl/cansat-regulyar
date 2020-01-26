/*
 * config.h
 *
 * Created: 24.03.2019 16:41:43
 *  Author: ThePetrovich
 */ 

#ifndef CONFIG_H_
#define CONFIG_H_

#define F_CPU 8000000L						//CPU frequency

//#define DEBUG 1

//------------------------Project-settings------------------------
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <util/delay.h>
#include "kernel/kernel.h"
#include "drivers/devices/sensors/ds18b20/ds18b20.h"
#include "drivers/interfaces/spi.h"
#include "drivers/interfaces/twi.h"
#include "drivers/devices/sensors/bmp280/bmp280.h"
#include "drivers/devices/sensors/dht11/DHT.h"
#include "drivers/devices/sensors/adxl345/adxl345.h"
//----------------------------------------------------------------

#endif /* CONFIG_H_ */