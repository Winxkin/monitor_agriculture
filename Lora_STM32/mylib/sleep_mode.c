#include "sleep_mode.h"

uint8_t sleepMode_init(TIM_HandleTypeDef *htim2)
{
	__HAL_RCC_TIM2_CLK_ENABLE(); //using TIM2
	
	TIM_ClockConfigTypeDef sClockSourceConfig;
  TIM_MasterConfigTypeDef sMasterConfig;

	
  htim2->Instance = TIM2;
  htim2->Init.Prescaler = 35999;
  htim2->Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2->Init.Period = 2500; // set 0.5s
  htim2->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2->Init.AutoReloadPreload=TIM_AUTORELOAD_PRELOAD_ENABLE;
	
	if (HAL_TIM_Base_Init(htim2) != HAL_OK)
  {
    return 0;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(htim2, &sClockSourceConfig) != HAL_OK)
  {
    return 0;
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(htim2, &sMasterConfig) != HAL_OK)
  {
    return 0;
  }
	
   HAL_NVIC_SetPriority(TIM2_IRQn,2, 0);
   HAL_NVIC_EnableIRQ(TIM2_IRQn);

	return 1;
}

void gotoSleepMode(TIM_HandleTypeDef *htim2)
{
	HAL_SuspendTick();
	//HAL_PWR_EnableSleepOnExit();
	HAL_TIM_Base_Start_IT(htim2);
	HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); // wait for interrupt
	HAL_ResumeTick();
	
}

