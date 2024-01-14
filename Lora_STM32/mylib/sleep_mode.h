#ifndef __SLEEP_MODE_H
#define __SLEEP_MODE_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"
#include "stm32f1xx_hal_rcc.h"

uint8_t sleepMode_init(TIM_HandleTypeDef *htim2);
void gotoSleepMode(TIM_HandleTypeDef *htim2);



#endif
