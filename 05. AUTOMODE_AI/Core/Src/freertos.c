/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os2.h"
#include "ultrasonic.h"        // US_Left_cm/Right/Center
#include "automode.h"          // 자동주행 상태 getter (아래 참고)
#include "stdio.h"
#include "move.h"
#include "robot_driver.h"
#include "bluetooth.h"
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
/* USER CODE BEGIN Variables */
extern RobotCar_t myRobot; // 전역 로봇 객체
extern volatile int debug_flag; // 가져오기
extern UART_HandleTypeDef huart1; // 블루투스 연결된 UART 번호

/* USER CODE END Variables */
/* Definitions for ultrasonic */
osThreadId_t ultrasonicHandle;
const osThreadAttr_t ultrasonic_attributes = {
  .name = "ultrasonic",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for autocontrol */
osThreadId_t autocontrolHandle;
const osThreadAttr_t autocontrol_attributes = {
  .name = "autocontrol",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void sonic(void *argument);
void automode(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of ultrasonic */
  ultrasonicHandle = osThreadNew(sonic, NULL, &ultrasonic_attributes);

  /* creation of autocontrol */
  autocontrolHandle = osThreadNew(automode, NULL, &autocontrol_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_sonic */
/**
  * @brief  Function implementing the ultrasonic thread.
  * @param  argument: Not used
  * @retval None
  */

/* USER CODE END Header_sonic */
void sonic(void *argument)
{
  /* USER CODE BEGIN sonic */
	osDelay(500);
  /* Infinite loop */
	  for(;;)
	  {
      // 1. 왼쪽
	  Sonar_Trigger(&sonarLeft);
	  osDelay(7); // 15ms 대기

	  // 2. 중앙
	  Sonar_Trigger(&sonarCenter);
	  osDelay(7);

	  // 3. 오른쪽
	  Sonar_Trigger(&sonarRight);
	  osDelay(7);
	  }
  /* USER CODE END sonic */
}

/* USER CODE BEGIN Header_automode */
/**
* @brief Function implementing the autocontrol thread.
* @param argument: Not used
* @retval None
*/
// [Actuator] 뇌의 명령(AutoOutput_t)을 하드웨어(BSP/Move)로 번역
/**
* @brief RNN AI 제어 및 블루투스 리포팅 태스크
*/
void AutoMode_Act(RobotCar_t *car, AutoOutput_t cmd)
{
	move_ai_independent(car, cmd.Speed_L, cmd.Speed_R, cmd.Dir_L, cmd.Dir_R);
}

/* USER CODE END Header_automode */
void automode(void *argument)
{
  /* USER CODE BEGIN automode */
    BT_Init(&huart1);
    AutoMode_Init();

    static int prev_dist_c = -1;
    uint32_t last_send_time = 0;

    osDelay(500);

    for(;;)
    {
        // 2. [SENSE]
        AutoInput_t input = {
            .Dist_L = (uint16_t)Sonar_Get_Distance(&sonarLeft),
            .Dist_R = (uint16_t)Sonar_Get_Distance(&sonarRight),
            .Dist_C = (uint16_t)Sonar_Get_Distance(&sonarCenter)
        };

        if (prev_dist_c == -1) prev_dist_c = input.Dist_C;

        // 3. [THINK]
        AutoOutput_t output = AutoMode_Run(input);

        // 4. [ACT]
        AutoMode_Act(&myRobot, output);

        // 5. [REPORT] (이건 그대로 50ms 유지 - 통신 과부하 방지)
        if (HAL_GetTick() - last_send_time >= 50)
        {
            BT_Telemetry_t pack;
            pack.Front_Dist = input.Dist_C;
            pack.Left_Dist  = input.Dist_L;
            pack.Right_Dist = input.Dist_R;
            pack.Delta_Front = (int)input.Dist_C - prev_dist_c;
            pack.L_PWM = output.Speed_L;
            pack.R_PWM = output.Speed_R;
            pack.L_Dir = (output.Dir_L == 1) ? BT_DIR_FWD : BT_DIR_STOP;
            pack.R_Dir = (output.Dir_R == 1) ? BT_DIR_FWD : BT_DIR_STOP;

            BT_Send_Telemetry(&btModule, &pack);

            prev_dist_c = input.Dist_C;
            last_send_time = HAL_GetTick();
        }

        // 6. [지연 최소화 핵심]
        // 기존 osDelay(20)은 너무 깁니다.
        // 1ms만 쉬게 해서 CPU 점유율은 양보하되, 가능한 가장 빠르게 다시 돕니다.
        osDelay(1);
    }
  /* USER CODE END automode */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

