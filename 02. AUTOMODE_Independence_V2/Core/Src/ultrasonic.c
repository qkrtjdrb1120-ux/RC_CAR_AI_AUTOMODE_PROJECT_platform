/*
 * ultrasonic.c
 *
 *  Created on: Nov 4, 2025
 *      Author: user13
 */

#include "ultrasonic.h"
#include <string.h> // memset 사용을 위해
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// ===== 튜닝/가드 =====
//[거리 필터링 규칙]
#define MAX_VALID_CM       400u   // 4m 넘으면 무효 (허공)
#define MIN_VALID_CM         2u   // 2cm 미만은 무효 (노이즈)
//[타이밍 규칙]
#define TRIG_GAP_MS         40u   // 센서 간 발사 간격

// 1. 전역 객체 생성 (배열 대신)
Sonar_t sonarLeft = {
    .timer_id    = BSP_TIMER_SONAR_LEFT,  // 이것만 있으면 bsp_timer.c가 알아서 처리함
    .trig_pin_id = BSP_PIN_TRIG_LEFT
};

Sonar_t sonarRight = {
    .timer_id    = BSP_TIMER_SONAR_RIGHT,
    .trig_pin_id = BSP_PIN_TRIG_RIGHT
};

Sonar_t sonarCenter = {
    .timer_id    = BSP_TIMER_SONAR_CENTER,
    .trig_pin_id = BSP_PIN_TRIG_CENTER
};

// [트리거 함수]
void Sonar_Trigger(Sonar_t *sonar)
{
    // 상태를 반드시 리셋해야 한다!
    sonar->state = 0;

    // 1. 플래그 클리어
    BSP_Timer_ClearFlag(sonar->timer_id);

    // 2. 엣지 설정 (Rising 감지부터 시작)
    BSP_Timer_SetEdge(sonar->timer_id, BSP_EDGE_RISING);

    // 3. 인터럽트 시작
    BSP_Timer_StartIC(sonar->timer_id);

    // 4. 트리거 펄스 발사
    BSP_GPIO_Write(sonar->trig_pin_id, false);
    BSP_Delay_us(1);
    BSP_GPIO_Write(sonar->trig_pin_id, true);
    BSP_Delay_us(10);
    BSP_GPIO_Write(sonar->trig_pin_id, false);
}

// [인터럽트 로직]
void Sonar_ISR_Process(Sonar_t *sonar)
{
    if (sonar->state == 0) {
        // [Rising Edge] 시작 시간 기록
//    	printf("ISR: Rising!\r\n"); // 이게 안 뜨면 선이 빠졌거나 핀 설정 오류
    	sonar->ic_start = BSP_Timer_GetCaptureValue(sonar->timer_id);
        sonar->state = 1;
        BSP_Timer_SetEdge(sonar->timer_id, BSP_EDGE_FALLING);
    }
    else if (sonar->state == 1) {
        // [Falling Edge] 종료 시간 기록
        sonar->ic_end = BSP_Timer_GetCaptureValue(sonar->timer_id);

        // 1. 시간 차이 계산 (STM32F4 TIM3/4는 16bit이므로 0xFFFF가 맞음)
        uint32_t diff;
        if (sonar->ic_end >= sonar->ic_start) {
            diff = sonar->ic_end - sonar->ic_start;
        } else {
            // Overflow 발생 시 보정 (16-bit Timer 가정)
            diff = (0xFFFF - sonar->ic_start) + sonar->ic_end + 1;
        }

        // 2. 거리 계산 (1us tick 가정: 340m/s / 2 / 10000 = 0.017)
        float temp_distance = (float)diff * 0.017f;
        // "누구냐 넌!"
//        char *name = "UNKNOWN";
//        if (sonar == &sonarLeft) name = "LEFT";
//        else if (sonar == &sonarRight) name = "RIGHT";
//        else if (sonar == &sonarCenter) name = "CENTER";
//
//        printf("[%s] Raw: %.2f cm (Diff: %lu)\r\n", name, temp_distance, diff);

        // 3. 필터링 (유효 범위 내일 때만 업데이트)
        if (temp_distance >= MIN_VALID_CM && temp_distance <= MAX_VALID_CM)
        {
            sonar->distance = temp_distance;
        }

        // 마무리
        BSP_Timer_StopIC(sonar->timer_id);
        sonar->state = 2; // 측정 완료 상태
    }
}

float Sonar_Get_Distance(Sonar_t *sonar)
{
	return (sonar -> distance);
}
