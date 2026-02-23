/*
 * bluetooth.h
 *
 *  Created on: Oct 28, 2025
 *      Author: user13
 */

#ifndef INC_BLUETOOTH_H_
#define INC_BLUETOOTH_H_

#include "main.h" // UART_HandleTypeDef 정의 포함
#include <stdint.h>

// [설정] 버퍼 크기 및 타임아웃
#define BT_TX_BUF_SIZE      128u
#define BT_TIMEOUT_MS       100u

// [방향 정의] move.c의 로직과 매핑하기 위한 상수
// DB에는 1=전진, 0=후진/정지 로 저장하기로 약속함
#define BT_DIR_FWD          1
#define BT_DIR_BWD          0

// 1. 전송 데이터 패킷 (DTO: Data Transfer Object)
typedef struct {
    // [센서 데이터]
    uint16_t Front_Dist;
    uint16_t Left_Dist;
    uint16_t Right_Dist;
    int      Delta_Front;   // 거리 변화량

    // [모터 데이터] (move.c의 동작 결과)
    uint8_t  L_Dir;         // 1:Forward, 0:Backward
    uint8_t  R_Dir;         // 1:Forward, 0:Backward
    uint16_t L_PWM;         // speed_left (0~1000)
    uint16_t R_PWM;         // speed_right (0~1000)
} BT_Telemetry_t;

// 2. 블루투스 객체 (Hardware Object)
// ultrasonic.c의 Sonar_t와 동일한 스타일
typedef struct {
    UART_HandleTypeDef *huart;         // UART 핸들
    uint8_t tx_buffer[BT_TX_BUF_SIZE]; // 내부 버퍼
    volatile uint8_t is_transmitting;  // 전송 중인지 확인하는 플래그 추가
} Bluetooth_t;

// [전역 객체 선언]
extern Bluetooth_t btModule;

// [함수 선언]
void BT_Init(UART_HandleTypeDef *huart_handle);
void BT_Send_Telemetry(Bluetooth_t *bt, BT_Telemetry_t *data);
void BT_Send_String(Bluetooth_t *bt, char *str); // 디버깅용

#endif /* INC_BLUETOOTH_H_ */
