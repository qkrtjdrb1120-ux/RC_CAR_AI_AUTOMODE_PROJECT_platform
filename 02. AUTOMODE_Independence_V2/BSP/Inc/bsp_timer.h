/*
 * bsp_timer.h
 *
 *  Created on: Dec 8, 2025
 *      Author: user
 */

#ifndef INC_BSP_TIMER_H_
#define INC_BSP_TIMER_H_

#include <stdint.h>
#include <stdbool.h>

// 1. 타이머 ID 정의 (하드웨어 핸들 대신 이걸 씀)
typedef enum {
    BSP_TIMER_SONAR_LEFT,   // TIM4 CH2
    BSP_TIMER_SONAR_RIGHT,  // TIM4 CH1
    BSP_TIMER_SONAR_CENTER  // TIM4 CH3
} bsp_timer_t;

// 2. 엣지 타입 정의 (Rising / Falling)
typedef enum {
    BSP_EDGE_RISING,
    BSP_EDGE_FALLING
} bsp_edge_t;

// 3. 함수 선언
// (1) 인터럽트와 카운터 시작
void BSP_Timer_StartIC(bsp_timer_t id);

// (2) 인터럽트 끄기
void BSP_Timer_StopIC(bsp_timer_t id);

// (3) 현재 캡처된 값 읽기 (IC Value)
uint32_t BSP_Timer_GetCaptureValue(bsp_timer_t id);

// (4) 감지할 엣지 변경 (Rising <-> Falling)
void BSP_Timer_SetEdge(bsp_timer_t id, bsp_edge_t edge);

// (5) 플래그 클리어 (필요하다면)
void BSP_Timer_ClearFlag(bsp_timer_t id);

void BSP_Delay_us(uint32_t us);

#endif /* INC_BSP_TIMER_H_ */
