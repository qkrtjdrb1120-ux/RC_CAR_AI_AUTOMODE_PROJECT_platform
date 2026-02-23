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

	// [초기화] 블루투스 시작
	BT_Init(&huart1);

	// 새로운 주행 시작 시 타임스탬프를 0으로 리셋 (추가 추천)
	BT_ResetTimestamp();

	// [변수 선언] 변화량 계산 및 100ms 주기 전송용
	// [변수 선언] 초기값을 -1로 해서 "아직 측정 안 함" 표시
	static int prev_dist_c = -1;
	uint32_t last_send_time = 0;

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

		// 2. 첫 루프라면 prev를 현재 값으로 셋팅 (변화량 0으로 시작)
		if (prev_dist_c == -1) {
			prev_dist_c = input.Dist_C;
		}

		// 3. [THINK] 찍어둔 사진(input)만 보고 고민한다
		// (센서는 이미 변했을지 몰라도, 뇌는 아까 찍은 사진만 보고 판단함 -> 논리적 오류 없음)
		AutoOutput_t output = AutoMode_Run(input);

		// 4. [ACT] 고민한 결과(output)대로 움직인다
		AutoMode_Act(&myRobot, output);

	    // ========================================================
	    // 4. [REPORT] 블루투스 데이터 전송 (추가된 부분)
	    // ========================================================
	    // 20ms마다 루프가 돌지만, 전송은 50ms(0.05초)마다 한 번씩 함 (데이터 폭주 방지)
	    if (HAL_GetTick() - last_send_time >= 50){
		    BT_Telemetry_t pack;

		    // (1) 센서 데이터 담기
		    pack.Front_Dist = input.Dist_C;
	  	    pack.Left_Dist  = input.Dist_L;
		    pack.Right_Dist = input.Dist_R;

		    // 전방 거리 변화량 (현재 - 과거)
		    // 첫 번째 전송 때 (200 - 200 = 0)이 되어 깔끔
		    pack.Delta_Front = (int)input.Dist_C - prev_dist_c;
		    prev_dist_c = input.Dist_C; // 다음을 위해 저장

		    // (2) 모터 PWM 담기 (automode.h의 결과값 그대로 사용)
		    pack.L_PWM = output.Speed_L;
		    pack.R_PWM = output.Speed_R;

		    // (3) 모터 방향 담기 (Action에 따라 변환)
		    // move.c 로직과 DB 약속(1=전진, 0=후진)에 맞춰 매핑
		    switch (output.Action) {
			    case ACTION_FORWARD:     // 둘 다 전진
				    pack.L_Dir = BT_DIR_FWD; pack.R_Dir = BT_DIR_FWD; break;
			    case ACTION_BACKWARD:    // 둘 다 후진
				    pack.L_Dir = BT_DIR_BWD; pack.R_Dir = BT_DIR_BWD; break;
			    case ACTION_PIVOT_RIGHT: // 좌:전진, 우:후진
				    pack.L_Dir = BT_DIR_FWD; pack.R_Dir = BT_DIR_BWD; break;
			    case ACTION_PIVOT_LEFT:  // 좌:후진, 우:전진
				    pack.L_Dir = BT_DIR_BWD; pack.R_Dir = BT_DIR_FWD; break;
			    default:                 // 정지
				    pack.L_Dir = BT_DIR_BWD; pack.R_Dir = BT_DIR_BWD; break;
		    }
		    // (4) 쏘세요!
		    BT_Send_Telemetry(&btModule, &pack);

		    // 시간 갱신
		    last_send_time = HAL_GetTick();
	    }

        osDelay(20); // 휴식

	//		[진단 코드]
    //		1. 센서 값과 뇌의 판단을 실시간으로 감시한다.
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

