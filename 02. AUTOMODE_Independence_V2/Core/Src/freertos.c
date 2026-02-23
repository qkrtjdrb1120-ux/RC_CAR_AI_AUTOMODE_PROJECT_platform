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
  .stack_size = 512 * 4,
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
	  osDelay(15); // 15ms 대기 (소리가 갔다가 돌아오고 잔향이 사라질 시간)

	  // 2. 중앙
	  Sonar_Trigger(&sonarCenter);
	  osDelay(15);

	  // 3. 오른쪽
	  Sonar_Trigger(&sonarRight);
	  osDelay(15);
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
void AutoMode_Act(RobotCar_t *car, AutoOutput_t cmd)
{
    // 1. [수리 완료] 속도 설정
    // speed.c가 사라졌으니 motor_recover도 없습니다.
    // 대신 구조체에 값을 넣어주면, move.c의 함수들이 움직일 때 이 속도를 가져다 씁니다.
    car->speed_left  = (uint16_t)cmd.Speed_L;
    car->speed_right = (uint16_t)cmd.Speed_R;

    // 만약 Robot_SetSpeed 함수를 만들었다면 그걸 써도 좋습니다.
    // Robot_SetSpeed(car, cmd.Speed_L, cmd.Speed_R);

    // 2. 행동(기어) 설정 (move.c 활용)
    switch (cmd.Action) {
        case ACTION_STOP:        motor_stop(car); break;
        case ACTION_FORWARD:     drive_forward(car); break;
        case ACTION_BACKWARD:    drive_backward(car); break;
        case ACTION_PIVOT_LEFT:  pivot_left(car); break;
        case ACTION_PIVOT_RIGHT: pivot_right(car); break;
        default:                 motor_stop(car); break;
    }
}

/* USER CODE END Header_automode */
void automode(void *argument)
{
  /* USER CODE BEGIN automode */
  /* Infinite loop */
	// 초기화 대기 (센서 안정화 등)
	osDelay(100);
  for(;;)
  {
  	// 1. [SENSE] 세상의 상태를 찰칵! 찍는다 (Input 생성)
		AutoInput_t input = {
				.Dist_L = (uint16_t)Sonar_Get_Distance(&sonarLeft),
				.Dist_R = (uint16_t)Sonar_Get_Distance(&sonarRight),
				.Dist_C = (uint16_t)Sonar_Get_Distance(&sonarCenter),
				.Current_Time_ms = HAL_GetTick()
		};

		// 2. [THINK] 찍어둔 사진(input)만 보고 고민한다
		// (센서는 이미 변했을지 몰라도, 뇌는 아까 찍은 사진만 보고 판단함 -> 논리적 오류 없음)
		AutoOutput_t output = AutoMode_Run(input);

		// 3. [ACT] 고민한 결과(output)대로 움직인다
		AutoMode_Act(&myRobot, output);

		osDelay(20); // 휴식

		// [진단 코드]
		// 1. 센서 값과 뇌의 판단을 실시간으로 감시한다.
//		printf("[DEBUG] Dist_C: %d cm | Action: %d (1:FWD, 2:BWD, 0:STOP)\r\n", input.Dist_C, output.Action);
//		printf("DebugFlag: %d\r\n", debug_flag);
//		printf("[Task] Addr: %p | Val: %u\r\n", (void*)&sonarCenter, input.Dist_C);
//		osDelay(100); // 로그가 너무 빠르면 보기 힘드니 0.1초로 늦춤

  }
  /* USER CODE END automode */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

