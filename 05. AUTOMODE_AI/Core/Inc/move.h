/*
 * move.h
 *
 *  Created on: Oct 28, 2025
 *      Author: user13
 */

#ifndef INC_MOVE_H_
#define INC_MOVE_H_

#include "robot_driver.h" // RobotCar_t 정의 포함

// 초기화 및 정지
void motor_init(RobotCar_t *car);
void motor_stop(RobotCar_t *car);

// 주행 동작
void drive_forward(RobotCar_t *car);
void drive_backward(RobotCar_t *car);
void pivot_right(RobotCar_t *car);
void pivot_left(RobotCar_t *car);

// [신규 추가] AI 전용 독립 제어 (Pre-heat 기능 포함)
// 기존 drive_forward 등을 건드리지 않으므로 안전함!
void move_ai_independent(RobotCar_t *car, uint16_t speed_l, uint16_t speed_r, uint8_t dir_l, uint8_t dir_r);

#endif /* INC_MOVE_H_ */
