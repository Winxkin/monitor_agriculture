#ifndef __RTC_SLEEPMODE_H
#define __RTC_SLEEPMODE_H
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_rcc.h"

uint8_t RTC_SleepMode_init(RTC_HandleTypeDef *hrtc);
uint8_t RTC_SetTime(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate);
#endif

