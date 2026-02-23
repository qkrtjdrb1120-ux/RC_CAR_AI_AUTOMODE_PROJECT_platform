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

// [콜백 함수 추가] 전송이 완료되면 호출됨
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
    if (huart == btModule.huart) {
        btModule.is_transmitting = 0; // 전송 완료 표시
    }
}

// [초기화] UART 핸들 연결
void BT_Init(UART_HandleTypeDef *huart_handle){
    btModule.huart = huart_handle;
    memset(btModule.tx_buffer, 0, BT_TX_BUF_SIZE);
}

// [텔레메트리 전송]
// 포맷: <F,L,R,dF,LD,RD,LP,RP>
void BT_Send_Telemetry(Bluetooth_t *bt, BT_Telemetry_t *data){
    // 방어 코드: 초기화 안 됐으면 무시
    if (bt->huart == NULL) return;

    // 이전 전송이 끝날 때까지 대기 (혹은 무시)
	// 115200에서는 매우 짧은 시간이지만 안정성을 위해 체크
    if (bt->is_transmitting) return;

    // 1. 데이터 포맷팅
    // move.c에서 결정된 속도(PWM)와 방향을 문자로 변환
    int len = snprintf((char *)bt->tx_buffer, BT_TX_BUF_SIZE, "<%d,%d,%d,%d,%d,%d,%d,%d>\n",
                      data->Front_Dist,
                      data->Left_Dist,
                      data->Right_Dist,
                      data->Delta_Front,
                      data->L_Dir,
                      data->R_Dir,
                      data->L_PWM,
                      data->R_PWM);

    // 2. UART 전송 (Blocking Mode)
    // 데이터 유실 방지를 위해 타임아웃 설정
    if (len > 0){
    	bt->is_transmitting = 1;
		// Blocking 방식 대신 DMA 방식 사용
		HAL_UART_Transmit_DMA(bt->huart, (uint8_t *)bt->tx_buffer, (uint16_t)len);
    }
}

// [디버깅용 문자열 전송]
void BT_Send_String(Bluetooth_t *bt, char *str){
    if (bt->huart == NULL) return;
    HAL_UART_Transmit(bt->huart, (uint8_t *)str, strlen(str), BT_TIMEOUT_MS);
}
