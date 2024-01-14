#include "LoRa.h"
/*define pin*/

#define M0 GPIO_PIN_11
#define M1 GPIO_PIN_12


int LORA_Init(UART_num UARTx,UART_HandleTypeDef *s_UARTHandle)
{
	GPIO_InitTypeDef GPIO_InitStructure;
		
	if(UARTx == UART1)
	{
		__USART1_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();
		GPIO_InitStructure.Pin = GPIO_PIN_9;
		GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
		HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);
		
		GPIO_InitStructure.Pin = GPIO_PIN_10;
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);
		
		s_UARTHandle->Instance = USART1;
	}
	else if(UARTx == UART2)
	{
		__USART2_CLK_ENABLE();
		__GPIOA_CLK_ENABLE();
		GPIO_InitStructure.Pin = GPIO_PIN_2;
		GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
		HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);
		
		GPIO_InitStructure.Pin = GPIO_PIN_3;
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA,&GPIO_InitStructure);
		
		s_UARTHandle->Instance = USART2;
	
	}
	else if(UARTx == UART3)
	{
		__USART3_CLK_ENABLE();
		__GPIOB_CLK_ENABLE();
		GPIO_InitStructure.Pin = GPIO_PIN_10;
		GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
		HAL_GPIO_Init(GPIOB,&GPIO_InitStructure);
		
		GPIO_InitStructure.Pin = GPIO_PIN_11;
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB,&GPIO_InitStructure);
		
		s_UARTHandle->Instance = USART3;
	}
  
	 HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
   HAL_NVIC_EnableIRQ(USART1_IRQn);

	s_UARTHandle->Init.BaudRate = 115200;
	s_UARTHandle->Init.WordLength = UART_WORDLENGTH_8B;
	s_UARTHandle->Init.StopBits = UART_STOPBITS_1;
	s_UARTHandle->Init.Parity = UART_PARITY_NONE;
	s_UARTHandle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	s_UARTHandle->Init.Mode = UART_MODE_TX_RX;
	s_UARTHandle->Init.OverSampling = UART_OVERSAMPLING_16;
	
	if(HAL_UART_Init(s_UARTHandle) == HAL_OK)
	{		

			return 1;
		 
	}
	else
	{
		  return 0;
	}
  
}

void Lora_SetMode (GPIO_TypeDef *GPIOx, E32_mode mode)
{
		GPIO_InitTypeDef  GPIO_InitStructure;
		
		if (GPIOx == GPIOA) 
		{
			__GPIOA_CLK_ENABLE();
	  } else if (GPIOx == GPIOB) 
		{
			__GPIOB_CLK_ENABLE();
		}
		
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
		
		GPIO_InitStructure.Pin = M0;
		HAL_GPIO_Init(GPIOx,&GPIO_InitStructure);
		
		GPIO_InitStructure.Pin = M1;
		HAL_GPIO_Init(GPIOx,&GPIO_InitStructure);
		
		//config mode
		switch (mode)
		{
			case mode0:
			{
					HAL_GPIO_WritePin(GPIOx, M0, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOx, M1, GPIO_PIN_RESET);
					break;
			}
			case mode1:
			{
					HAL_GPIO_WritePin(GPIOx, M0, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOx, M1, GPIO_PIN_RESET);
					break;
			}
			case mode2:
			{
					HAL_GPIO_WritePin(GPIOx, M0, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOx, M1, GPIO_PIN_SET);
					break;
			}
			case mode3:
			{
					HAL_GPIO_WritePin(GPIOx, M0, GPIO_PIN_SET);
					HAL_GPIO_WritePin(GPIOx, M1, GPIO_PIN_SET);
					break;
			}
			default:
			{
					break;
			}
		}
		HAL_Delay(500);
		
}

void Lora_transmit ( UART_HandleTypeDef *s_UARTHandle, uint8_t *pData, option _option)
{		
		uint16_t lengh;
	  if(_option == _string)
		{
			lengh = strlen((char*)pData);
		}
		if(_option == _struct)
		{
			lengh = sizeof(pData);
		}
		lengh = lengh +1;
		HAL_UART_Transmit(s_UARTHandle,pData,lengh,HAL_MAX_DELAY);
}

int Lora_Receive ( UART_HandleTypeDef *s_UARTHandle, uint8_t *pData)
{		
		uint16_t lengh = sizeof(pData);
		if(HAL_UART_Receive(s_UARTHandle,pData,lengh,HAL_MAX_DELAY) == HAL_OK)
		{
			return 1;
		}
		else
		{
			return 0;
		}
}

void clear_data(uint8_t *pData)
{
	 for(int i = 0; i < sizeof(pData) ; i++)
		{
				pData[i] = 0x00;
		}
	
}

uint8_t checksum (uint8_t *Data, int lengh)
{
  int sum = 0;  
  for(int i =0; i<lengh ; i++)
  {
    sum += Data[i];
  }
  return (~sum + 2);
}

uint8_t checksum_rxData (uint8_t *Data, int lengh)
{
  uint8_t sum = 0;  
  for(int i =0; i<lengh ; i++)
  {
    sum += Data[i];
  }
  return (~sum + 2);
}

