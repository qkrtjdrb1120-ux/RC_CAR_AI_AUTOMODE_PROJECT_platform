/*
 * bluetooth.c
 *
 *  Created on: Oct 28, 2025
 *      Author: user13
 */
#include "bluetooth.h"
#include <stdio.h>  // sprintf
#include <string.h> // strlen, memset

// 1. 전역 객체 생성
Bluetooth_t btModule;

// 2. 주행 시작 시간 관리를 위한 변수
static uint32_t start_tick = 0;
static uint8_t is_first_run = 1;

// [콜백 함수] 전송 완료 표시
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    if (huart == btModule.huart) {
        btModule.is_transmitting = 0;
    }
}

// [초기화] UART 핸들 연결
void BT_Init(UART_HandleTypeDef *huart_handle){
    btModule.huart = huart_handle;
    memset(btModule.tx_buffer, 0, BT_TX_BUF_SIZE);
}

// [텔레메트리 전송] 수정된 부분
// 포맷: <Timestamp,F,L,R,dF,LD,RD,LP,RP>
void BT_Send_Telemetry(Bluetooth_t *bt, BT_Telemetry_t *data){

    if (bt->huart == NULL) return;
    if (bt->is_transmitting) return;

    // 3. 첫 전송 시 현재 시간을 시작 지점으로 저장
    if (is_first_run) {
        start_tick = HAL_GetTick(); // 전원 켜진 후 흐른 ms
        is_first_run = 0;
    }

    // 4. 현재 시간에서 시작 시간을 빼서 '상대 시간' 계산
    uint32_t timestamp_ms = HAL_GetTick() - start_tick;

    uint32_t checksum = timestamp_ms + data->Front_Dist + data->Left_Dist + data->Right_Dist +
                        data->Delta_Front + data->L_Dir + data->R_Dir + data->L_PWM + data->R_PWM;

    // 5. 데이터 포맷팅: 맨 앞에 %lu (Unsigned Long) 추가
    // 8개 데이터 -> 9개 데이터로 변경됨
    // 마지막에 %u로 checksum 추가 (10번째 데이터)
    int len = snprintf((char *)bt->tx_buffer, BT_TX_BUF_SIZE, "<<%lu,%d,%d,%d,%d,%d,%d,%d,%d,%u>>\n",
                      timestamp_ms, data->Front_Dist, data->Left_Dist, data->Right_Dist,
                      data->Delta_Front, data->L_Dir, data->R_Dir, data->L_PWM, data->R_PWM,
                      (unsigned int)(checksum % 256)); // 보통 1바이트(0~255)로 보냅니다.

    if (len > 0){
        bt->is_transmitting = 1;
        HAL_UART_Transmit_DMA(bt->huart, (uint8_t *)bt->tx_buffer, (uint16_t)len);
    }
}

// [디버깅용 문자열 전송]
void BT_Send_String(Bluetooth_t *bt, char *str){
    if (bt->huart == NULL) return;
    HAL_UART_Transmit(bt->huart, (uint8_t *)str, strlen(str), BT_TIMEOUT_MS);
}

// 새로운 주행을 시작할 때 시간을 0ms로 다시 맞추고 싶다면 호출
void BT_ResetTimestamp(void) {
    // 이 함수를 쓰려면 start_tick과 is_first_run을
    // 함수 밖(파일 상단)으로 빼서 static 전역 변수로 만들어야 합니다.
    is_first_run = 1;
}
