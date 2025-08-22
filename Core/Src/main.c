/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "ST7920_lib.h"
#include "logo.h"
#include "morse_decoder.h"
#include "debounce.h"

extern char tx_buffer[128];

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//If you hold the button longer than this time, it will be a dash, otherwise it will be a dot
#define DOT_MAX_TIME 450

#define CHECK_MSEC    5		//Timer period
#define PRESS_MSEC    50	//Press debounce time
#define RELEASE_MSEC  50	//Release debounce time

//A new character is registered after 1 second of inactivity (200 timer activations).
#define NEW_SYMBOL_CNT	(1000 / CHECK_MSEC)

//A new word after 3 seconds
#define NEW_WORD_CNT	(3000 / CHECK_MSEC)

#define LEFT_MARGIN 1	//How many pixels to indent horizontally in front of the text
#define TOP_MARGIN 1	//How many lines to indent vertically in front of the text

#define DECODED_MAX_LINES 2 //How many lines the decoded text can take up
#define MORSE_MAX_LINES 4	//How many lines the morse text can take up

#define CHARS_PER_LINE 21	//How many characters can be placed in a line

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim1;

/* USER CODE BEGIN PV */

//The character that the user is currently typing
MorseChar_t current_char = {0, 0};

//String with decoded morse text
char decoded_text[CHARS_PER_LINE * DECODED_MAX_LINES + 1] = "\0";	//String with decoded text

//Morse string with dots and dashes
char morse_text[CHARS_PER_LINE * MORSE_MAX_LINES + 1] = "\0";	//String with dots and dashes

//Information about button
Button_t morse_btn;
uint32_t press_time = 0;

//Position of the flashing cursor
uint8_t cursor_x = LEFT_MARGIN;
uint8_t cursor_y = TOP_MARGIN;

bool should_draw = false;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

void set_last_char(char* str, char c, size_t max_length) {
	size_t current_length = strlen(str);

	//If the string is full - make it empty
    if (current_length >= max_length - 1) {
        str[0] = '\0';
        current_length = 0;
    }

    str[current_length] = c;
    str[current_length + 1] = '\0';
}

//Check in the timer for a new char/word
void new_char_or_word(uint32_t *char_count, uint32_t *word_count) {

	//Timer expired - record the current character
	if (*char_count != 0 && (--(*char_count) == 0)) {
		char decoded_char = decode_morse_char(&current_char);

		if (decoded_char == '\0') {
			decoded_char = '?';
		}

		set_last_char(decoded_text, decoded_char, sizeof(decoded_text));
		set_last_char(morse_text, ' ', sizeof(morse_text));

		//Reset current char
		current_char = (MorseChar_t){0, 0};

		should_draw = true;
	}

	//Timer expired - add a space
	if (*word_count != 0 && (--(*word_count) == 0)) {
		set_last_char(decoded_text, ' ', sizeof(decoded_text));
		set_last_char(morse_text, ' ', sizeof(morse_text));

		should_draw = true;
	}
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */

  __HAL_TIM_CLEAR_FLAG(&htim1, TIM_SR_UIF);

  HAL_Delay(100);
  ST7920_Init();
  ST7920_Graphic_mode(1);

  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  draw_logo();

  ST7920_Draw_rectangle_filled(0, 0, 127, 63);
  ST7920_Update();

  init_button(&morse_btn, BTN_GPIO_Port, BTN_Pin, CHECK_MSEC, PRESS_MSEC, RELEASE_MSEC, true);

  //Start a timer with main logic
  HAL_TIM_Base_Start_IT(&htim1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  static uint32_t last_cursor_draw_time = 0;
	  static bool cursor_state = false;

	  //Draw cursors every 500 ms
	  if (HAL_GetTick() - last_cursor_draw_time >= 500) {
		  last_cursor_draw_time = HAL_GetTick();

		  cursor_state = !cursor_state;

		  if (cursor_state) {
			  for (int i = 0; i < 8; i++) {
				  ST7920_Clean_pixel(cursor_x, cursor_y * 8 + i);
			  }
		  } else {
			  for (int i = 0; i < 8; i++) {
				  ST7920_Draw_pixel(cursor_x, cursor_y * 8 + i);
			  }
		  }

		  ST7920_Update();
	  }

	  if (should_draw) {

		  ST7920_Draw_rectangle_filled(0, 0, 127, 63);

		  cursor_x = LEFT_MARGIN;
		  cursor_y = TOP_MARGIN;

		  uint8_t len = strlen(decoded_text);
		  //Print decoded text
		  for (int i = 0; i < DECODED_MAX_LINES; i++) {
			  if (len <= CHARS_PER_LINE * i) {
				  break;
			  }


			  //Print up to 21 characters per line (move the rest to the next line)
			  uint8_t segment_len = (len >= CHARS_PER_LINE * (i + 1)) ? CHARS_PER_LINE : len - CHARS_PER_LINE * i;
			  strncpy(tx_buffer, decoded_text + (CHARS_PER_LINE * i), segment_len);
			  tx_buffer[segment_len] = '\0';
			  ST7920_Decode_UTF8(LEFT_MARGIN, TOP_MARGIN + i, 1, tx_buffer);

			  //Move the cursor position to the position of the next character
			  if (segment_len == CHARS_PER_LINE) {
				  cursor_x = LEFT_MARGIN;
				  cursor_y++;
			  } else {
				  cursor_x = LEFT_MARGIN + (segment_len) * 6;
			  }
		  }

		  len = strlen(morse_text);
		  //Print morse text
		  for (int i = 0; i < MORSE_MAX_LINES; i++) {
			  if (len <= CHARS_PER_LINE * i) {
				  break;
			  }

			  //Print up to 21 characters per line (move the rest to the next line)
			  uint8_t segment_len = (len >= CHARS_PER_LINE * (i + 1)) ? CHARS_PER_LINE : len - CHARS_PER_LINE * i;
			  strncpy(tx_buffer, morse_text + (CHARS_PER_LINE * i), segment_len);
			  tx_buffer[segment_len] = '\0';
			  ST7920_Decode_UTF8(LEFT_MARGIN, TOP_MARGIN + 2 + i, 1, tx_buffer);
		  }

		  ST7920_Update();

		  should_draw = false;
	  }

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 7200;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 50;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, CS_Pin|RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_Pin RST_Pin */
  GPIO_InitStruct.Pin = CS_Pin|RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_Pin */
  GPIO_InitStruct.Pin = BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) {

    	//Get debounced data
        bool key_changed = false;
        bool key_pressed = false;
        debounce_button(&morse_btn, &key_changed, &key_pressed);

        //Set the timer for new word or character
        static uint32_t char_count = 0;
        static uint32_t word_count = 0;
        new_char_or_word(&char_count, &word_count);

        if (key_changed) {
        	key_changed = false;

			if (key_pressed) {
				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
				press_time = HAL_GetTick() - PRESS_MSEC;

				char_count = 0;
				word_count = 0;
			} else {
				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

				//Hold the button longer than DOT_MAX_TIME: dash
				if (HAL_GetTick() - RELEASE_MSEC - press_time >= DOT_MAX_TIME) {
					add_unit(&current_char, true);
					set_last_char(morse_text, '-', sizeof(morse_text));
				} else { //less than DOT_MAX_TIME: dot
					add_unit(&current_char, false);
					set_last_char(morse_text, '.', sizeof(morse_text));
				}

				should_draw = true;

				//Reset the timer for a new character or word
				char_count = NEW_SYMBOL_CNT;
				word_count = NEW_WORD_CNT;
			}
        }
    }
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
