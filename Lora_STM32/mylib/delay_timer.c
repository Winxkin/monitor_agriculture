/*****************************************************************************************************
@tac gia: khuenguyen
@website: khuenguyencreator.com
@youtube: https://www.youtube.com/channel/UCt8cFnPOaHrQXWmVkk-lfvg
@huong dan su dung:
- Khoi tao timer dem moi 1us VD: fapb2 = 72 mhz. prescaler = 72 -1, ARR = 0xFFFF -1 
- Truyen timer do vao delay Init VD: DELAY_TIM_Init(&htim1)
- Su dung thu vien hal

*****************************************************************************************************/
#include "delay_timer.h"
uint8_t DELAY_TIM_Init(TIM_HandleTypeDef *htim4)
{
	 /* USER CODE BEGIN TIM4_Init 0 */
  __HAL_RCC_TIM4_CLK_ENABLE();
  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4->Instance = TIM4;
  htim4->Init.Prescaler = 72-1;
  htim4->Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4->Init.Period = 0xffff - 1;
  htim4->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(htim4) != HAL_OK)
  {
    return 0;
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(htim4, &sClockSourceConfig) != HAL_OK)
  {
    return 0;
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(htim4, &sMasterConfig) != HAL_OK)
  {
    return 0;
  }
  /* USER CODE BEGIN TIM4_Init 2 */
	HAL_TIM_Base_Start(htim4);
	return 1;
  /* USER CODE END TIM4_Init 2 */
}

void DELAY_TIM_Us(TIM_HandleTypeDef *htim4, uint16_t time)
{
	__HAL_TIM_SET_COUNTER(htim4,0);
	while(__HAL_TIM_GET_COUNTER(htim4)<time){}
}
void DELAY_TIM_Ms(TIM_HandleTypeDef *htim4, uint16_t Time)
{
	__HAL_TIM_SET_COUNTER(htim4,0);
	while(Time--)
	{
		while(__HAL_TIM_GET_COUNTER(htim4)<1000){}
	}
}
