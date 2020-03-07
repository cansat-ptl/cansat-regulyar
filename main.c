/*
 * main.c
 *
 * Created: 07.03.2019 17:56:34
 *  Author: ThePetrovich
 */ 

#include "config.h"

void ds18b20rd(){
	ds18b20_requestTemperature();
	delay_ms(1000);
	char * str = ds18b20_readTemperature();
	debug_logMessage(PGM_ON, L_NONE, PSTR("temperature: %s\r\n"), str);
}

void bmp280rd(){
	
	float t = bmp280_readTemperature();
	float prs = bmp280_readPressure();
	debug_logMessage(PGM_ON, L_NONE, PSTR("t1: %f, p: %f\r\n"), t, prs);
}

void adxl345rd(){
	float x = adxl345_readX();
	float y = adxl345_readY();
	float z = adxl345_readZ();
	debug_logMessage(PGM_ON, L_NONE, PSTR("x1: %f, y1: %f,z1: %f\r\n"), x, y, z);
}
void dht22rd(){
	double t2[1];
	double hum[1];
	DHT_GetHumidity(hum);
	debug_logMessage(PGM_ON, L_NONE, PSTR("hum: %f, t2: %f\r\n"), hum[0], t2[0]);
}

int main(void){
	
	hal_enableInterrupts();
	wdt_enable(WDTO_2S);
	spi_init();
	hal_setupPins();
	
	kernel_addTask(KTASK_REPEATED, ds18b20rd, 1000, KPRIO_LOW, KSTATE_ACTIVE);
	kernel_addTask(KTASK_REPEATED, bmp280rd, 1000, KPRIO_HIGH, KSTATE_ACTIVE);
	kernel_addTask(KTASK_REPEATED, dht22rd, 1000, KPRIO_LOW, KSTATE_ACTIVE);
	kernel_addTask(KTASK_REPEATED, adxl345rd, 100, KPRIO_HIGH, KSTATE_ACTIVE);
	kernelInit();
}
