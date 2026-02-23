/*
 * move.c
 *
 *  Created on: Oct 28, 2025
 *      Author: user13
 */

#include "move.h"

// [초기화] 모터를 정지 상태로 설정
void motor_init(RobotCar_t *car)
{
    motor_stop(car);
}

// [정지]
void motor_stop(RobotCar_t *car)
{
    BSP_Motor_Drive(car->motor_id_left, car->speed_left, BSP_MOTOR_DIR_STOP);
    BSP_Motor_Drive(car->motor_id_right, car->speed_right, BSP_MOTOR_DIR_STOP);
}

// [전진]
void drive_forward(RobotCar_t *car)
{
    BSP_Motor_Drive(car->motor_id_left, car->speed_left, BSP_MOTOR_DIR_FORWARD);
    BSP_Motor_Drive(car->motor_id_right, car->speed_right, BSP_MOTOR_DIR_FORWARD);
}

// [후진]
void drive_backward(RobotCar_t *car)
{
    BSP_Motor_Drive(car->motor_id_left, car->speed_left, BSP_MOTOR_DIR_BACKWARD);
    BSP_Motor_Drive(car->motor_id_right, car->speed_right, BSP_MOTOR_DIR_BACKWARD);
}

// [제자리 우회전] (Pivot)
void pivot_right(RobotCar_t *car)
{
    // 왼쪽: 전진, 오른쪽: 후진
    BSP_Motor_Drive(car->motor_id_left, car->speed_left, BSP_MOTOR_DIR_FORWARD);
    BSP_Motor_Drive(car->motor_id_right, car->speed_right, BSP_MOTOR_DIR_BACKWARD);
}

// [제자리 좌회전] (Pivot)
void pivot_left(RobotCar_t *car)
{
    // 왼쪽: 후진, 오른쪽: 전진
    BSP_Motor_Drive(car->motor_id_left, car->speed_left, BSP_MOTOR_DIR_BACKWARD);
    BSP_Motor_Drive(car->motor_id_right, car->speed_right, BSP_MOTOR_DIR_FORWARD);
}

void move_ai_independent(RobotCar_t *car, uint16_t speed_l, uint16_t speed_r, uint8_t dir_l, uint8_t dir_r)
{
    // 상태 기록 (로깅용)
    car->speed_left = speed_l;
    car->speed_right = speed_r;

    // 왼쪽 바퀴: 방향(1/0)에 따라 Forward/Stop 결정하되, Speed는 유지
    if (dir_l) {
        BSP_Motor_Drive(car->motor_id_left, speed_l, BSP_MOTOR_DIR_FORWARD);
    } else {
        // [현업의 디테일] 멈추지만 에너지는 장전(Pre-heat)
        BSP_Motor_Drive(car->motor_id_left, speed_l, BSP_MOTOR_DIR_STOP);
    }

    // 오른쪽 바퀴
    if (dir_r) {
        BSP_Motor_Drive(car->motor_id_right, speed_r, BSP_MOTOR_DIR_FORWARD);
    } else {
        BSP_Motor_Drive(car->motor_id_right, speed_r, BSP_MOTOR_DIR_STOP);
    }
}
