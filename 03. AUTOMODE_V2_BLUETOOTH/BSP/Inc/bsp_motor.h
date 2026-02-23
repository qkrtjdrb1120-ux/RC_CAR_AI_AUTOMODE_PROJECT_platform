/*
 * bsp_motor.h
 *
 *  Created on: Dec 8, 2025
 *      Author: user
 */

#ifndef INC_BSP_MOTOR_H_
#define INC_BSP_MOTOR_H_

#include <stdint.h>

// 1. 모터 ID 정의 (추상화)
typedef enum {
    BSP_MOTOR_LEFT,
    BSP_MOTOR_RIGHT
} bsp_motor_t;

// 2. 모터 방향 정의
typedef enum {
    BSP_MOTOR_DIR_FORWARD,
    BSP_MOTOR_DIR_BACKWARD,
    BSP_MOTOR_DIR_STOP
} bsp_motor_dir_t;

// 3. 함수 선언
// (1) 모터 초기화 (PWM 시작 등)
void BSP_Motor_Init(void);

// (2) 속도 및 방향 설정
// speed: 0 ~ 1000 (PWM 듀티)
// dir: 전진/후진/정지
void BSP_Motor_Drive(bsp_motor_t motor, uint16_t speed, bsp_motor_dir_t dir);

#endif /* INC_BSP_MOTOR_H_ */
