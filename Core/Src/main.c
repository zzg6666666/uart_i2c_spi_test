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
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "uart_printf.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define AHT20 0x38
#define BMP280 0x77 << 1
#define AT24C256 0xA0

#define AT24C256_NORMAL_TEST 1
#define AT24C256_IT_TEST 0
#define AT24C256_DMA_TEST 0
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  // 初始化HAL库
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  // 配置system clock pll HCLK PCLK1 PCK2
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  // 初始化DMA
  MX_DMA_Init();

  MX_I2C1_Init();
  // 初始化串口
  MX_USART1_UART_Init();
  // 初始化SPI
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  char TxData[] = "TEST FOR AT24C256 \r\n";

  // HAL_UART_Transmit_IT(&huart1, (uint8_t *)TxData, strlen(TxData));
  uart_printf_init();

  while (1)
  {
    /* USER CODE END WHILE */
    // HAL_UART_Transmit_IT(&huart1, (uint8_t *)TxData, strlen(TxData));
    uart_printf(TxData, NULL, 0);
    HAL_Delay(1000);

    uint8_t I2CTXdata[] = {0x00, 0x0F, 0xE2, 0xA3, 0xB8, 0x77, 0xfa, 0xaE, 0xBA};
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
#if AT24C256_NORMAL_TEST
    uart_printf("NORMAL TEST FOR AT24C256 \r\n", NULL, 0);

    uart_printf("AT24C256 write data:", &I2CTXdata[1], 8);

    HAL_I2C_Master_Transmit(&hi2c1, AT24C256, I2CTXdata, 9, 100);

    HAL_Delay(100);
    uint8_t I2CRXdata[8] = {};
    HAL_I2C_Master_Transmit(&hi2c1, AT24C256, I2CTXdata, 1, 100);
    HAL_I2C_Master_Receive(&hi2c1, AT24C256, &I2CRXdata[0], 8, 100);

    uart_printf("AT24C256 read data:", &I2CRXdata[0], 8);
#endif
    // HAL_I2C_Master_Receive(&hi2c1, BMP280, &data, 1, 100);
    // uart_printf("i2c rceive data", &data);
    //  uint8_t buffdata[1000];
    // uart_fifo_get(&buffdata[0], sizeof(buffdata));
    // HAL_Delay(1000);
    // HAL_UART_Transmit(&huart1, buffdata, sizeof(buffdata), 100);
    /* USER CODE BEGIN 3 */
    // HAL_I2C_Master_Transmit(&hi2c1, 0x38, (uint8_t*)TxData, strlen(TxData), 100);

    // HAL_I2C_Master_Transmit_DMA(&hi2c1, 0x38,(uint8_t*)TxData,strlen(TxData));
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  // 内部/外部振荡器结构体
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  // RCC系统、AHB总线和APB总线的时钟配置结构定义
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  // 配置时钟结构体
  // 配置外置时钟
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  // 开启外置时钟
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  // HSE时钟预分频器 设置为不分频 RCC_CFGR bit17
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  // 开启内置时钟
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  // 使用PLL
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  // PLL时钟源是HSE
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  // PLL倍频9倍
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;

  // 配置HSE、PLL
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  // 使能HCLK、SYSCLK、PCLK1、PCLK2
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  // 系统时钟使用PLL
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  // AHB预分频器不分频
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  // APB1预分频器2分频
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  // APB2预分频器不分频
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
