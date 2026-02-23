/*
 * bsp_motor.c
 *
 *  Created on: Dec 8, 2025
 *      Author: user
 */

#include "bsp_motor.h"
#include "main.h" // htim3, GPIO 핀 정의 등을 위해 필요

extern TIM_HandleTypeDef htim3; // 모터 타이머

// 오른쪽 모터 (IN1, IN2)
#define IN1_PIN  	GPIO_PIN_4
#define IN1_GPIO_PORT  	GPIOA
#define IN2_PIN  	GPIO_PIN_0
#define IN2_GPIO_PORT  	GPIOB
// 왼쪽 모터 (IN3, IN4)
#define IN3_PIN  	GPIO_PIN_1
#define IN3_GPIO_PORT  	GPIOC
#define IN4_PIN  	GPIO_PIN_0
#define IN4_GPIO_PORT  	GPIOC

// [내부 함수] 방향 핀 설정 (L298N 드라이버 기준)
static void Set_Direction(bsp_motor_t motor, bsp_motor_dir_t dir) {
    GPIO_TypeDef *port1, *port2;
    uint16_t pin1, pin2;

    // 1. 모터에 따른 핀 매핑 (main.h에 정의된 매크로 사용)
    if (motor == BSP_MOTOR_RIGHT) {
        // IN1, IN2
        port1 = IN1_GPIO_PORT; pin1 = IN1_PIN;
        port2 = IN2_GPIO_PORT; pin2 = IN2_PIN;
    } else {
        // IN3, IN4
        port1 = IN3_GPIO_PORT; pin1 = IN3_PIN;
        port2 = IN4_GPIO_PORT; pin2 = IN4_PIN;
    }

    // 2. 방향에 따른 핀 상태 설정
    if (dir == BSP_MOTOR_DIR_FORWARD) {
        HAL_GPIO_WritePin(port1, pin1, GPIO_PIN_SET);
        HAL_GPIO_WritePin(port2, pin2, GPIO_PIN_RESET);
    } else if (dir == BSP_MOTOR_DIR_BACKWARD) {
        HAL_GPIO_WritePin(port1, pin1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(port2, pin2, GPIO_PIN_SET);
    } else { // STOP
        HAL_GPIO_WritePin(port1, pin1, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(port2, pin2, GPIO_PIN_RESET);
    }
}

// [공개 함수] 초기화
void BSP_Motor_Init(void) {
    // PWM 시작
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // Right
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); // Left
}

// [공개 함수] 구동
void BSP_Motor_Drive(bsp_motor_t motor, uint16_t speed, bsp_motor_dir_t dir) {
    // 1. 방향 핀 설정
    Set_Direction(motor, dir);

    // 2. PWM 속도 설정
    uint32_t channel = (motor == BSP_MOTOR_LEFT) ? TIM_CHANNEL_2 : TIM_CHANNEL_1;
    __HAL_TIM_SET_COMPARE(&htim3, channel, speed);
}
