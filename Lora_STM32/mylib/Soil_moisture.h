#ifndef __SOIL_MOISTURE_H
#define __SOIL_MOISTURE_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"


/*This library design for module soil_moisture*/
/*A0 -> PA0 */
/*analog value 0->1023*/
uint8_t Sensor_init_ADC1(ADC_HandleTypeDef *hadc1,DMA_HandleTypeDef *hdma_adc1);
uint8_t Soil_moisture_Read(ADC_HandleTypeDef *hadc1,uint16_t var);
uint8_t MQ135_Read(ADC_HandleTypeDef *hadc1,uint16_t var);
uint8_t Light_Read(ADC_HandleTypeDef *hadc1,uint16_t var);

void get_sensor_data(ADC_HandleTypeDef *hadc1,uint8_t *humi,uint8_t *light,uint8_t *air);

#endif
