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

#endif /* INC_MOVE_H_ */
