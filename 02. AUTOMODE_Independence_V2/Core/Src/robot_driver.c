/*
 * robot_driver.c
 *
 *  Created on: Nov 30, 2025
 *      Author: user13
 */

// robot_driver.c (새로 만들 파일)

#include "robot_driver.h" // RobotCar_t 정의가 있는 곳

// [속도 설정]
// 예전: 레지스터에 값을 직접 씀 (즉시 반영)
// 지금: 구조체 변수에 값을 저장 (Move 함수 호출 시 반영)
void Robot_SetSpeed(RobotCar_t *car, int left_speed, int right_speed)
{
    // 1. 구조체에 '희망 속도'를 저장해둔다.
    car->speed_left  = (uint16_t)left_speed;
    car->speed_right = (uint16_t)right_speed;

    // (참고: 만약 즉시 반영하고 싶다면, 현재 방향을 알거나
    //  MoveForward 등을 여기서 바로 호출해야 하는데,
    //  보통은 제어 루프(Task)에서 Move 함수를 계속 부르므로 저장만 해도 충분.)
}

// [전진]
// 예전: HAL_GPIO_WritePin으로 핀 4개를 지지고 볶음
// 지금: "왼쪽 모터 전진! 오른쪽 모터 전진!" 명령만 내림
void Robot_MoveForward(RobotCar_t *car)
{
    // 2. 저장된 속도(speed_left/right)를 가지고 '전진' 명령 수행
    // 타이머 채널이 뭔지, 핀 번호가 뭔지는 BSP가 다 알아서 함!
    BSP_Motor_Drive(car->motor_id_left,  car->speed_left,  BSP_MOTOR_DIR_FORWARD);
    BSP_Motor_Drive(car->motor_id_right, car->speed_right, BSP_MOTOR_DIR_FORWARD);
}
