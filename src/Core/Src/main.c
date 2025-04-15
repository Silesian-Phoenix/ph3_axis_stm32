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
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <json.h>
#include "stdio.h"
#include "stepper_motor.h"
#include "stdbool.h"
#include "string.h"
#include "ee.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define AS5600_BUF_SIZE 3
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

  /*enkoder*/
  uint8_t as5600_buf[3];
  uint8_t ENCODER_buf[3];
  HAL_StatusTypeDef ENCODER_i2c_status;
  uint16_t ENCODER_data;
  double ENCODER_current_angle = 0.0;
  uint16_t ENCODER_recal_data;
  uint16_t ENCODER_offset = 0;

bool addr_to_send = false;
bool addr_to_set = false;
int received_addr = false;



  bool ENCODER_init = true;
  bool i2c_data_to_calc = false;
  bool eeprom_data_to_read = true;
bool reset_enable = false;
bool set_enable = false;

  /*JSON*/
  bool uart2_data_received = false;
  bool uart2_tx_busy = false;
  bool data_to_send = false;

  typedef struct {
    uint16_t EE_offset;
  uint16_t addr;
  } eeStorage_t;

  eeStorage_t ee;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// printf 
int __io_putchar(int ch) {
  if (ch == '\n') {
    __io_putchar('\r');
  }
  HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
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
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  /*JSON*/
  lwjson_my_init();
  HAL_UARTEx_ReceiveToIdle_IT(&huart2, (uint8_t*)rx_buffer, sizeof(rx_buffer));

  /*ENKODER*/
  HAL_I2C_Mem_Read_DMA(&hi2c1, 0x36 << 1, 0x0B, 1, as5600_buf, 3);
  
  EE_Init(&ee, sizeof(eeStorage_t)); 

  HAL_GPIO_WritePin(MOTOR_ENABLE_GPIO_Port, MOTOR_ENABLE_Pin, GPIO_PIN_SET);
  while (1)
  { 
    if ((as5600_buf[0] & (1<<5)) == 0) {
      HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
      HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
    }
    else {
      HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
    }

    if (data_to_send) {
      char buf_to_send[50];
      sprintf(buf_to_send, "angle: %.2f", MOTOR_current_angle);
      if (!uart2_tx_busy) {
          HAL_UART_Transmit_DMA(&huart2, (uint8_t*)buf_to_send, strlen(buf_to_send));
          uart2_tx_busy = true;
      }
      data_to_send = false;
    }
  
    // otrzymano pozycję która wychodzi po za margines błędu obecnej -> zmiana położenia
    if (MOTOR_current_status == MOTOR_ANGLE_RECEIVED && accept_margin(MOTOR_current_angle, MOTOR_target_angle, MOTOR_PROPER_ANGLE_MARGIN) != 1) {
      MOTOR_state_machine(MOTOR_current_angle, MOTOR_target_angle, MOTOR_ANGLE_RECEIVED);

      // char buf_to_send[50];
      // sprintf(buf_to_send, "angle: %d", MOTOR_target_angle);
      // if (!uart2_tx_busy) {
      //     HAL_UART_Transmit_DMA(&huart2, (uint8_t*)buf_to_send, strlen(buf_to_send));
      //     uart2_tx_busy = true;
      // }
    }

    // otrzymano pozycje i obecna miesci sie w zakresie otrzymanej -> nie zmieniamy położenia
    else if (MOTOR_current_status == MOTOR_ANGLE_RECEIVED && accept_margin(MOTOR_current_angle, MOTOR_target_angle, MOTOR_PROPER_ANGLE_MARGIN)) {
      MOTOR_state_machine(MOTOR_current_angle, MOTOR_target_angle, MOTOR_ANGLE_RECEIVED);
    }
    
    // silnik w trakcie ustawiania na pozycje
    else if (MOTOR_current_status == MOTOR_IN_MOTION) {
      MOTOR_state_machine(MOTOR_current_angle, MOTOR_target_angle, MOTOR_IN_MOTION);
    }

    // silnik na pozycji
    else if (MOTOR_current_status == MOTOR_AT_POSITION) {
      MOTOR_state_machine(MOTOR_current_angle, MOTOR_target_angle, MOTOR_AT_POSITION);
    }

    // silnik na pozycji i w obrębie marginesu
    else if (MOTOR_current_status == MOTOR_OUT) {
      MOTOR_state_machine(MOTOR_current_angle, MOTOR_target_angle, MOTOR_OUT);
    }
    
    

    // setAdr
    if (addr_to_set)
    {
      ee.addr = received_addr;
      EE_Write();
      addr_to_set = false;
    }

    // getAdr
    if (addr_to_send)
    {
      EE_Read();
      char buf_to_send[50];
      sprintf(buf_to_send, "{\"addr\": %d}", ee.addr);
      if (!uart2_tx_busy)
      {
        HAL_UART_Transmit_DMA(&huart2, (uint8_t *)buf_to_send, strlen(buf_to_send));
        uart2_tx_busy = true;
      }
      addr_to_send = false;
    }


    if (set_enable)
    {
      set_enable = false;
      HAL_GPIO_WritePin(MOTOR_ENABLE_GPIO_Port, MOTOR_ENABLE_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
    }

    if (reset_enable)
    {
      reset_enable = false;
      HAL_GPIO_WritePin(MOTOR_ENABLE_GPIO_Port, MOTOR_ENABLE_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */


/*ODBIERANIE DANYCH W PRZERWANIU*/
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
  /*JSON*/
  if (huart == &huart2) {
      rx_buffer[Size] = '\0';
      uart2_data_received = true;

      if (uart2_data_received) {
        json_process(rx_buffer);
        uart2_data_received = false;
      }
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, (uint8_t*)rx_buffer, sizeof(rx_buffer));
  }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart == &huart2) {
    uart2_tx_busy = false;
  }
}


// wersja z zapisem do eeprom
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
  if (hi2c == &hi2c1) {
    // obliczenie kąta
    ENCODER_data = ((as5600_buf[1] << 8) | as5600_buf[2]); // poskładanie danych z enkodera
    if (ENCODER_init == false) {
      ENCODER_offset = ENCODER_data; // zapisanie offsetu
      ee.EE_offset = ENCODER_offset;
      EE_Write();
      ENCODER_init = true;
      eeprom_data_to_read = false;
      MOTOR_target_angle = 0;
    }
    else {
      if (eeprom_data_to_read) {
        EE_Read();
        ENCODER_offset = ee.EE_offset;
        eeprom_data_to_read = false;
      }
      else {
        ENCODER_recal_data = (ENCODER_data - ENCODER_offset + 4095) % 4095;
        ENCODER_current_angle = (double)ENCODER_recal_data * 0.08789;
        MOTOR_current_angle = ENCODER_current_angle;
      }
    }
    
    // sprawdzenie statusu I2C
    if (HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_READY) {
      HAL_I2C_Mem_Read_DMA(&hi2c1, 0x36 << 1, 0x0B, 1, as5600_buf, 3);
    }
  }
}

// wersja bez zapisy do eeprom
// void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
//   if (hi2c == &hi2c1) {
//     // obliczenie kąta
//     ENCODER_data = ((as5600_buf[1] << 8) | as5600_buf[2]); // poskładanie danych z enkodera
//     if (ENCODER_init == false) {
//       ENCODER_offset = ENCODER_data; // zapisanie offsetu
//       ENCODER_init = true;
//     } else {
//       ENCODER_recal_data = (ENCODER_data - ENCODER_offset + 4095) % 4095;
//       ENCODER_current_angle = (double)ENCODER_recal_data * 0.08789;
//       MOTOR_current_angle = ENCODER_current_angle;
//     }

//     // sprawdzenie statusu I2C
//     if (HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_READY) {
//       HAL_I2C_Mem_Read_DMA(&hi2c1, 0x36 << 1, 0x0B, 1, as5600_buf, 3);
//     }
//   }
// }


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
