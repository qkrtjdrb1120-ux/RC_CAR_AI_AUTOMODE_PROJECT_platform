/*
 * robot_driver.h
 *
 *  Created on: Nov 30, 2025
 *      Author: user13
 */

#ifndef INC_ROBOT_DRIVER_H_
#define INC_ROBOT_DRIVER_H_

#include "bsp_motor.h" // 추가

typedef struct {
    // 하드웨어 정보는 이제 ID만 있으면 됨!
    bsp_motor_t motor_id_left;
    bsp_motor_t motor_id_right;

    // 상태 변수 (현재 속도 등)는 유지
    uint16_t speed_left;
    uint16_t speed_right;
} RobotCar_t;

void Robot_SetSpeed(RobotCar_t *car, int left_speed, int right_speed);
void Robot_MoveForward(RobotCar_t *car);

#endif /* INC_ROBOT_DRIVER_H_ */
