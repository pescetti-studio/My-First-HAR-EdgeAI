/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "lsm6dsox_reg.h"
#include "model.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LSM6DSOX_I2C_ADDR8    (0x6B << 1)

#define PI 	3.14159265358979323846		// Costante pi greco
#define G 	9.80665						// Accelerazione di gravità

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

// Output
extern uint8_t predicted_class;
extern uint8_t confidence;

// Media e deviazione standard per ogni feature
const float mean[6] = {-0.3109933f, -0.21435975f, 4.9039526f, 0.12456421f, 0.06937116f, -0.03399972f};
const float std[6]  = {84.00599f, 140.4055f, 224.9723f, 18.348589f, 17.98235f, 16.988403f};

static char tx_line[64];
static stmdev_ctx_t dev_ctx;

float input_buffer[100][6];    	// Buffer per 2s di dati
uint8_t sample_index = 0;    	// Contatore di campioni

// Funzioni platform per driver ST
static int32_t platform_write(void *handle, uint8_t reg,
                              const uint8_t *bufp, uint16_t len)
{
    return (HAL_I2C_Mem_Write((I2C_HandleTypeDef*)handle,
                              LSM6DSOX_I2C_ADDR8, reg,
                              I2C_MEMADD_SIZE_8BIT,
                              (uint8_t*)bufp, len, 1000) == HAL_OK) ? 0 : 1;
}

static int32_t platform_read(void *handle, uint8_t reg,
                             uint8_t *bufp, uint16_t len)
{
    return (HAL_I2C_Mem_Read((I2C_HandleTypeDef*)handle,
                             LSM6DSOX_I2C_ADDR8, reg,
                             I2C_MEMADD_SIZE_8BIT,
                             bufp, len, 1000) == HAL_OK) ? 0 : 1;
}


static void platform_delay(uint32_t ms) { HAL_Delay(ms); }
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

// Funzione di conversione da dps a rad/sec
float conv_dps2rad (float in_dps) {
	float out_rad = in_dps * PI / 180;
	return out_rad;
}

// Funzione di conversione da g a m/s^2
float conv_g2ms2 (float in_g) {
	float out_ms2 = in_g * G;
	return out_ms2;
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
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg  = platform_read;
  dev_ctx.handle    = &hi2c1;

  platform_delay(100);

  // WHO AM I
  uint8_t who_am_i = 0;
  int32_t status = lsm6dsox_device_id_get(&dev_ctx, &who_am_i);
  snprintf(tx_line, sizeof(tx_line), "WHO_AM_I letto: 0x%02X, status: %ld\r\n", who_am_i, status);
  HAL_UART_Transmit(&huart2, (uint8_t*)tx_line, strlen(tx_line), HAL_MAX_DELAY);

  // Sensor reset
  lsm6dsox_reset_set(&dev_ctx, PROPERTY_ENABLE);
  uint8_t rst;
  do { lsm6dsox_reset_get(&dev_ctx, &rst); } while (rst);

  // Configuration
  lsm6dsox_i3c_disable_set(&dev_ctx, LSM6DSOX_I3C_DISABLE);
  lsm6dsox_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

  // ODR 52 Hz
  lsm6dsox_xl_data_rate_set(&dev_ctx, LSM6DSOX_XL_ODR_52Hz);
  lsm6dsox_gy_data_rate_set(&dev_ctx, LSM6DSOX_GY_ODR_52Hz);

  // Full scale ±8 g and ±2000 dps
  lsm6dsox_xl_full_scale_set(&dev_ctx, LSM6DSOX_8g);
  lsm6dsox_gy_full_scale_set(&dev_ctx, LSM6DSOX_2000dps);

  // Data buffers
  int16_t acc_raw[3], gyr_raw[3];
  float   acc_mg[3],  gyr_mdps[3];
  uint32_t t_prev = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	      /* USER CODE END WHILE */

	      /* USER CODE BEGIN 3 */

			// Wait 20 ms for 50 Hz sampling
	  while ((HAL_GetTick() - t_prev) < 20) {}
	  t_prev = HAL_GetTick();

	  lsm6dsox_status_reg_t status;
	  lsm6dsox_status_reg_get(&dev_ctx, &status);
	  if (!(status.xlda && status.gda))
		  continue;

	  lsm6dsox_acceleration_raw_get(&dev_ctx, acc_raw);
	  lsm6dsox_angular_rate_raw_get(&dev_ctx, gyr_raw);

	  for (int i = 0; i < 3; i++) {
		  acc_mg[i] = lsm6dsox_from_fs8_to_mg(acc_raw[i]);
		  gyr_mdps[i] = lsm6dsox_from_fs2000_to_mdps(gyr_raw[i]);
	  }

	    /*
	    // Print inertial data at 50 Hz frequency
	    snprintf(tx_line, sizeof(tx_line),
	             "Acc: %.2f %.2f %.2f, Gyro: %.2f %.2f %.2f\r\n",
	             conv_g2ms2(acc_mg[0]/1000.0f), conv_g2ms2(acc_mg[1]/1000.0f), conv_g2ms2(acc_mg[2]/1000.0f),
	             conv_dps2rad(gyr_mdps[0]/1000.0f), conv_dps2rad(gyr_mdps[1]/1000.0f), conv_dps2rad(gyr_mdps[2]/1000.0f));
	    HAL_UART_Transmit(&huart2, (uint8_t*)tx_line, strlen(tx_line), HAL_MAX_DELAY);
	    */

		// Inertial data for the input buffer
	  if (sample_index < 100) {
		  input_buffer[sample_index][0] = conv_g2ms2(acc_mg[0] / 1000.0f);
		  input_buffer[sample_index][1] = conv_g2ms2(acc_mg[1] / 1000.0f);
		  input_buffer[sample_index][2] = conv_g2ms2(acc_mg[2] / 1000.0f);
		  input_buffer[sample_index][3] = conv_dps2rad(gyr_mdps[0] / 1000.0f);
		  input_buffer[sample_index][4] = conv_dps2rad(gyr_mdps[1] / 1000.0f);
		  input_buffer[sample_index][5] = conv_dps2rad(gyr_mdps[2] / 1000.0f);
		  sample_index++;
	  }

	  if (sample_index >= 100) {
		  sample_index = 0;  // Reset buffer

		  // Normalize the 100 samples
		  float normalized_buffer[100][6];
		  for (int i = 0; i < 100; i++) {
			  for (int j = 0; j < 6; j++) {
				  normalized_buffer[i][j] = (input_buffer[i][j] - mean[j]) / std[j];
			  }
		  }

		  // Inference on acquired and normalized data
		  lstm_inference(normalized_buffer);

		  // Array for class names
		  const char* class_names[] = { "Downstairs", "Running", "Standing", "Upstairs", "Walking", "Unknown" };
		  const char* current_class_name;
		  if (predicted_class < 5) {
			  current_class_name = class_names[predicted_class];
		  }
		  else {
			  current_class_name = class_names[5];
		  }

		  // Build the formatted string
		  snprintf(tx_line, sizeof(tx_line),
			  "%ACTIVITY: %s\t\tCONFIDENCE: %u%%\r\n",
			  current_class_name, confidence);
		  HAL_UART_Transmit(&huart2, (uint8_t*)tx_line, strlen(tx_line), HAL_MAX_DELAY);
	  }

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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

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
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
