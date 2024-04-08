/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <stdlib.h>
#include <stdbool.h>//使用bool类型，需要加入该�?
#include <stdint.h>//使用bool类型，需要加入该�?
#define Input_1 HAL_GPIO_ReadPin( Input_1_GPIO_Port, Input_1_Pin)
#define Input_2 HAL_GPIO_ReadPin( Input_2_GPIO_Port, Input_2_Pin)
#define Input_3 HAL_GPIO_ReadPin( Input_3_GPIO_Port, Input_3_Pin)
#define Input_4 HAL_GPIO_ReadPin( Input_4_GPIO_Port, Input_4_Pin)
#define Input_5 HAL_GPIO_ReadPin( Input_5_GPIO_Port, Input_5_Pin)
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

/* USER CODE BEGIN PV */

//状�?�变�?
bool pi_Straight = false;
bool pi_Stop = false;
bool pi_Left = false;
bool pi_Right = false;
int Count = 0;
bool Straight = false;
bool Stop = false;
bool weiLeft = false;
bool weiRight = false;
bool Left = false;
bool Right = false;
char Car_State;
int count = 0;
int count_temp,delta;
int fputc(int ch, FILE *f)
{
	 
		HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
	 
		return ch;
	 
}
int fgetc(FILE *f)
{
  uint8_t ch = 0;
  HAL_UART_Receive(&huart1, &ch, 1, 0xffff);
  return ch;
}
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
struct _pid{
	float SetSpeed;//定义设定�?
	float ActualSpeed;//定义实际�?
	float err;//定义偏差�?
	float err_next;//定义上一个偏差�??
	float err_last;//定义�?上前的偏差�??
	float Kp, Ki, Kd;//定义比例、积分�?�微分系�?
}pid;
float Speed;
//PID初始�?
void PID_init(){
	pid.SetSpeed = 0.0;
	pid.ActualSpeed = 0.0;
	pid.err = 0.0;
	pid.err_last = 0.0;
	pid.err_next = 0.0;
	//可设置下面的值，从�?�达到最佳效�?
	pid.Kp = 0.2;
	pid.Ki = 0.04; 
	pid.Kd = 0.2;
}

float PID_realize(float speed ,float ActualSpeed){
	float incrementSpeed = 100;
	pid.ActualSpeed = ActualSpeed;
	pid.SetSpeed = speed;
	pid.err = pid.SetSpeed - pid.ActualSpeed;
  incrementSpeed = pid.Kp*(pid.err - pid.err_next) + pid.Ki*pid.err + pid.Kd*(pid.err - 2 * pid.err_next + pid.err_last);//计算出增�?
	pid.ActualSpeed += incrementSpeed;
	pid.err_last = pid.err_next;
	pid.err_next = pid.err;
	return incrementSpeed;
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
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);//启动PWM信号通道
HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
//__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_2,0);
HAL_TIM_Base_Start_IT(&htim3);
HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
PID_init();
Speed = 0;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		//接收串口信息
		HAL_UART_Receive_IT(&huart1,(uint8_t *)&Car_State,1);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);//控制正转反转
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);//控制正转反转
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
		//读取相应的巡线信息
		//直行
		if(Input_1 == 1 && Input_2 == 1 && Input_3 == 0 && Input_4 == 1 && Input_5 == 1)
		{
			Straight = true;
			Stop = false;
			Left = false;
			Right = false;
			weiLeft = false;
			weiRight = false;
		}
		//左转
		else if((Input_1 == 1 || Input_2 == 1)&&( Input_4 == 0 && Input_5==0))
		//else if((Input_1 == 1 && Input_2 == 1)&&( Input_4 == 0 || Input_5==0))
		{
			Straight = false;
			Stop = false;
			Left = false;
			Right = true;
			weiLeft = false;
			weiRight = false;
		}
		//右转
		else if((Input_1 == 0 && Input_2 == 0)&&(Input_4 == 1 || Input_5 == 1))
		//else if((Input_1 == 0 || Input_2 == 0)&&(Input_4 == 1&&Input_5==1))
		{
			Straight = false;
			Stop = false;
			Left = true;
			Right = false;
			weiLeft = false;
			weiRight = false;
		}
		else if((Input_1 == 1 || Input_2 == 1)&&( Input_4 == 0))
		{
			Straight = false;
			Stop = false;
			Left = false;
			Right = false;
			weiLeft = false;
			weiRight = true;
		}
		else if((Input_2 == 0)&&(Input_4 == 1 || Input_5 == 1))
		//else if((Input_1 == 0 || Input_2 == 0)&&(Input_4 == 1&&Input_5==1))
		{
			Straight = false;
			Stop = false;
			Left = false;
			Right = false;
			weiLeft = true;
			weiRight = false;
		}
		else if(Input_1 == 0 && Input_2 == 0 && Input_3 == 0 && Input_4 == 0 && Input_5 == 0)
		{
			if(pi_Straight == 1)
			{
				Count++;
			}
			if(pi_Straight == 1 && Count >60000)
			{
				Straight = false;
				Stop = false;
				Left = false;
				Right = true;
			}else
			{
				Straight = pi_Straight;
				Stop = pi_Stop;
				Left = pi_Left;
				Right = pi_Right;
			}
			
			//Straight = true;
			//Stop = false;
			//Left = false;
			//Right = false;
			weiLeft = false;
			weiRight = false;
		}

		if(Straight)
		{
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_2,100);//右轮
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_1,100);//左轮
			//Straight = false;
		}
		else if(Stop)
		{
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_2,0);//右轮
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_1,0);//左轮
			//Stop = false;
		}
		else if(Left)
		{
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_2,10);//右轮
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_1,100);//左轮
			//Left = false;
		}
		else if(Right)
		{
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_2,100);//右轮
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_1,10);//左轮
			//Right = false;
		}
		else if(weiLeft)
		{
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_2,50);//右轮
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_1,100);//左轮
		}
		else if(weiRight)
		{
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_2,100);//右轮
			__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_1,50);//左轮
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

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
	{

      if(htim == (&htim3))
      { 
				count_temp = count;
				count = (uint32_t)(__HAL_TIM_GET_COUNTER(&htim4));
				pid.ActualSpeed = count_temp - count;
					if(Straight)
					{
							printf("Straight\r\n");
					}
					else if(Stop)
					{
							printf("Stop\r\n");
					}
					else if(Left)
					{
							printf("Left\r\n");
					}
					else if(Right)
					{
							printf("Right\r\n");
					}
				//printf("%d  \n",(int)pid.ActualSpeed);
				//printf("%d\r\n",Input_1&Input_2);
				//printf("%d%d%d%d%d\r\n",Input_1,Input_2,Input_3,Input_4,Input_5);
				//printf("%d  \n",(int)PID_realize(100,pid.ActualSpeed));
//				if((int)PID_realize(100,pid.ActualSpeed)<0)
//				__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_2,0); 
//				else
//				__HAL_TIM_SET_COMPARE (&htim2, TIM_CHANNEL_2,(int)PID_realize(100,pid.ActualSpeed));
				
			}
	}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	switch(Car_State)
	{
		case '0':
			pi_Straight = true;
			pi_Stop = false;
			pi_Left = false;
			pi_Right = false;
			printf("0--Straight  %d\n   ",Count);//换成上述定义的bool类型即可
		break;
		case '1':
			pi_Straight = false;
			pi_Stop =true;
			pi_Left = false;
			pi_Right = false;
			printf("1--Stop  %d\n   ",Count);
		break;
		case '2':
			pi_Straight = false;
			pi_Stop = false;
			pi_Left = true;
			pi_Right = false;
			printf("2--Left  %d\n   ",Count);
		break;
		case '3':
			pi_Straight = false;
			pi_Stop = false;
			pi_Left = false;
			pi_Right = true;
			printf("3--Right  %d\n   ",Count);
		break;
		default:
			break;
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
