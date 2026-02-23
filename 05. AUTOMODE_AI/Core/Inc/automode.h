/*
 * automode.h
 *
 *  Created on: Nov 4, 2025
 *      Author: user13
 */

#ifndef INC_AUTOMODE_H_
#define INC_AUTOMODE_H_

#include "main.h"

#define RNN_WINDOW_SIZE 10
#define RNN_INPUT_DIM   3

typedef struct {
    uint16_t Dist_L;
    uint16_t Dist_C;
    uint16_t Dist_R;
} AutoInput_t;

typedef struct {
    uint16_t Speed_L;  // PWM 값 (0~1000)
    uint16_t Speed_R;  // PWM 값 (0~1000)

    // [필수 추가]
    // 좌우 바퀴 각각 전진(1)인지 정지(0)인지 구분해야 합니다.
    uint8_t  Dir_L;    // 1: GO, 0: STOP (PWM 유지)
    uint8_t  Dir_R;    // 1: GO, 0: STOP (PWM 유지)

    uint8_t  Action;   // 로깅용 (전진/정지/회전 등 상태 표시)
} AutoOutput_t;

void AutoMode_Init(void);
AutoOutput_t AutoMode_Run(AutoInput_t input);

#endif
