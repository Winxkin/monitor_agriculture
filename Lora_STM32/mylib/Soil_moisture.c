#include "Soil_moisture.h"


uint8_t Sensor_init_ADC1(ADC_HandleTypeDef *hadc1,DMA_HandleTypeDef *hdma_adc1)
{	
		__HAL_RCC_ADC1_CLK_ENABLE();
	  __HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_DMA1_CLK_ENABLE();
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		
	
	  ADC_ChannelConfTypeDef sConfig = {0};
		hadc1->Instance = ADC1;
		hadc1->Init.ScanConvMode = ADC_SCAN_ENABLE;
		hadc1->Init.ContinuousConvMode = DISABLE;
		hadc1->Init.DiscontinuousConvMode = DISABLE;
		hadc1->Init.ExternalTrigConv = ADC_SOFTWARE_START;
		hadc1->Init.DataAlign = ADC_DATAALIGN_RIGHT;
		hadc1->Init.NbrOfConversion = 3;
  if (HAL_ADC_Init(hadc1) != HAL_OK)
  {
    return 0;
  }
	
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
  if (HAL_ADC_ConfigChannel(hadc1, &sConfig) != HAL_OK)
  {
    return 0;
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(hadc1, &sConfig) != HAL_OK)
  {
    return 0;
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(hadc1, &sConfig) != HAL_OK)
  {
    return 0;
  }

	  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
	/* ADC1 DMA Init */
    /* ADC1 Init */
   
		
	
	return 1;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) 
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint8_t Soil_moisture_Read(ADC_HandleTypeDef *hadc1,uint16_t var)
{

	  uint8_t humi = map(var,0,4095,1,100);
		return (100 - humi); // return humi % value
}

uint8_t MQ135_Read(ADC_HandleTypeDef *hadc1,uint16_t var)
{

	  uint8_t air = map(var,0,4095,1,100);
		return air; // return air % value -> 0% -> good 100% -> bad
}

uint8_t Light_Read(ADC_HandleTypeDef *hadc1,uint16_t var)
{
	  uint8_t light = map(var,0,4095,1,100);
		return (100-light); // return air % value -> 0% -> good 100% -> bad
}

void get_sensor_data(ADC_HandleTypeDef *hadc1,uint8_t *humi,uint8_t *light,uint8_t *air)
{
		uint16_t var[3];
	  HAL_ADC_Start_DMA(hadc1,(uint32_t*)var,3);
	  HAL_Delay(100);
	  /*PA0 -> humi*/
		*light = Light_Read(hadc1,var[0]);
		/*PA1 -> light*/
		*humi = Soil_moisture_Read(hadc1,var[1]);
	  /*PA2 -> air*/
		//*air = var[2];
}



