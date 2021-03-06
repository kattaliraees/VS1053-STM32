/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "VS1053.h"
#include "testmp3.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//For writing data to VS10xx registers at addr
void vs1053_write_sci(uint8_t addr, uint16_t data) {

	uint8_t d = 2;

	while(!HAL_GPIO_ReadPin(VS1053_DREQ_GPIO_Port, VS1053_DREQ_Pin)); //Wait until DREQ is high

	HAL_GPIO_WritePin(VS1053_CS_GPIO_Port, VS1053_CS_Pin, 0); //Activate CS (Chip select, Active Low)
	HAL_SPI_Transmit(&hspi2, &d, 1, 50); //Write command code
	HAL_SPI_Transmit(&hspi2, &addr, 1, 50); //SCI Register number
	d = ((uint8_t)(data >> 8) & 0xFF);
	HAL_SPI_Transmit(&hspi2, &d, 1, 50); //LSB
	d = ((uint8_t)(data & 0xFF));
	HAL_SPI_Transmit(&hspi2, &d, 1, 50); //LSB
	HAL_GPIO_WritePin(VS1053_CS_GPIO_Port, VS1053_CS_Pin, 1); //De-Activate CS (Chip select, Active Low)
}

//For reading VS10xx register at addr
uint16_t vs1053_read_sci(uint8_t addr) {

	uint16_t res;

	uint8_t r;

	uint8_t d = 3;

	while(!HAL_GPIO_ReadPin(VS1053_DREQ_GPIO_Port, VS1053_DREQ_Pin)); //Wait until DREQ is high


	HAL_GPIO_WritePin(VS1053_CS_GPIO_Port, VS1053_CS_Pin, 0); //Activate CS (Chip select, Active Low)
	HAL_SPI_Transmit(&hspi2, &d, 1, 50); //Read command code
	HAL_SPI_Transmit(&hspi2, &addr, 1, 50); //SCI Register number
	HAL_SPI_Receive(&hspi2, &r, 1, 50);
	res = ((uint16_t)(r << 8)) & 0xFF00;
	HAL_SPI_Receive(&hspi2, &r, 1, 50);
	res |= ((uint16_t)r & 0x00FF);
	HAL_GPIO_WritePin(VS1053_CS_GPIO_Port, VS1053_CS_Pin, 1); //De-Activate CS (Chip select, Active Low)

	return res;
}


//Sending MP3 raw bytes. Max 32 bytes at time without checking DREQ each time
int vs1053_write_sdi(uint8_t *data, uint8_t bytes){


	if(bytes > 32) {
		return -1;//Error - too many bytes to transfer
	}

	while(!HAL_GPIO_ReadPin(VS1053_DREQ_GPIO_Port, VS1053_DREQ_Pin));


	HAL_GPIO_WritePin(VS1053_DCS_GPIO_Port, VS1053_DCS_Pin, 0); //De-Activate DCS (Active Low)


	HAL_SPI_Transmit(&hspi2, data, bytes, 50);
	data++;

	HAL_GPIO_WritePin(VS1053_DCS_GPIO_Port, VS1053_DCS_Pin, 1);
	return 0;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */


  //VS1053 init
  HAL_GPIO_WritePin(VS1053_CS_GPIO_Port, VS1053_CS_Pin, 1); // cs_high();  //MP3_XCS, Init Control Select to deselected
  HAL_GPIO_WritePin(VS1053_DCS_GPIO_Port, VS1053_DCS_Pin, 1); //dcs_high(); //MP3_XDCS, Init Data Select to deselected
  HAL_GPIO_WritePin(VS1053_RST_GPIO_Port, VS1053_RST_Pin, 0); //digitalWrite(MP3_RESET, LOW); //Put VS1053 into hardware reset
  HAL_Delay(5);
  HAL_GPIO_WritePin(VS1053_RST_GPIO_Port, VS1053_RST_Pin, 1);
  /////

  uint16_t data = vs1053_read_sci(SCI_MODE);

  if(data != (SM_LINE1 | SM_SDINEW)) {

	  vs1053_write_sci(SCI_MODE, (SM_LINE1 | SM_SDINEW));
  }

  //Sending test mp3 data
  int j = 0;
  for(int i = outputfile_mp3_len; i > 32; i = i-32) {

	  vs1053_write_sdi(&outputfile_mp3[j], 32);
	  j = j+32;

  }




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, VS1053_RST_Pin|VS1053_CS_Pin|VS1053_DCS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : VS1053_RST_Pin VS1053_CS_Pin VS1053_DCS_Pin */
  GPIO_InitStruct.Pin = VS1053_RST_Pin|VS1053_CS_Pin|VS1053_DCS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : VS1053_DREQ_Pin */
  GPIO_InitStruct.Pin = VS1053_DREQ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VS1053_DREQ_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
