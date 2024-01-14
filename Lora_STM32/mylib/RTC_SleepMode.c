#include "RTC_SLeepMode.h"

uint8_t RTC_SleepMode_init(RTC_HandleTypeDef *hrtc)
{
		 HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
		__HAL_RCC_AFIO_CLK_ENABLE();
		__HAL_RCC_PWR_CLK_ENABLE();

  /* System interrupt init*/

  /* Peripheral interrupt init */
  /* RCC_IRQn interrupt configuration */
		HAL_NVIC_SetPriority(RCC_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(RCC_IRQn);
	
		/** Initialize RTC Only*/
		hrtc->Instance = RTC;
		hrtc->Init.AsynchPrediv = RTC_AUTO_1_SECOND;
		hrtc->Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
		if (HAL_RTC_Init(hrtc) != HAL_OK)
		{
			return 0;
		}
		
		return 1;

}

uint8_t RTC_SetTime(RTC_HandleTypeDef *hrtc, RTC_TimeTypeDef *sTime, RTC_DateTypeDef *sDate)
{
	sTime->Hours = 0x9;
	sTime->Minutes = 0x1;
	sTime->Seconds = 0x0;
  if (HAL_RTC_SetTime(hrtc, sTime, RTC_FORMAT_BCD) != HAL_OK)
	{
	    return 0;
	}
	sDate->WeekDay = RTC_WEEKDAY_TUESDAY;
	sDate->Month = RTC_MONTH_FEBRUARY;
	sDate->Date = 0x23;
	sDate->Year = 0x21;
	if (HAL_RTC_SetDate(hrtc, sDate, RTC_FORMAT_BCD) != HAL_OK)
	{
	    return 0;
	}
	
	return 1;

}
