/*
 * bsp_timer.c
 *
 *  Created on: Dec 8, 2025
 *      Author: user
 */

#include "bsp_timer.h"
#include "main.h" // htim4를 쓰기 위해 필요!
#include "ultrasonic.h"

// 외부에서 정의된 핸들 가져오기
extern TIM_HandleTypeDef htim4;  // 초음파용
extern TIM_HandleTypeDef htim11; // 딜레이용

// [내부 도우미 함수] ID를 받아서 Channel 상수를 리턴
static uint32_t Get_Channel(bsp_timer_t id) {
    switch(id) {
        case BSP_TIMER_SONAR_LEFT:   return TIM_CHANNEL_2;
        case BSP_TIMER_SONAR_RIGHT:  return TIM_CHANNEL_1;
        case BSP_TIMER_SONAR_CENTER: return TIM_CHANNEL_3;
        default: return 0;
    }
}

// 1. 시작 함수
void BSP_Timer_StartIC(bsp_timer_t id) {
    HAL_TIM_IC_Start_IT(&htim4, Get_Channel(id));
}

// 2. 정지 함수 (인터럽트 끄기)
void BSP_Timer_StopIC(bsp_timer_t id) {
    HAL_TIM_IC_Stop_IT(&htim4, Get_Channel(id));
    // 또는 __HAL_TIM_DISABLE_IT(&htim4, Get_Channel(id));
}

// 3. 값 읽기
uint32_t BSP_Timer_GetCaptureValue(bsp_timer_t id) {
    return HAL_TIM_ReadCapturedValue(&htim4, Get_Channel(id));
}

// 4. 엣지 변경 (이제 여기서 다 처리!)
void BSP_Timer_SetEdge(bsp_timer_t id, bsp_edge_t edge) {
    uint32_t polarity = (edge == BSP_EDGE_RISING) ? TIM_INPUTCHANNELPOLARITY_RISING : TIM_INPUTCHANNELPOLARITY_FALLING;
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim4, Get_Channel(id), polarity);
}

// 5. 플래그 클리어
void BSP_Timer_ClearFlag(bsp_timer_t id) {
    // 채널별 플래그 클리어는 매크로가 좀 복잡하니,
    // 여기서는 간단히 전체 클리어하거나 필요한 플래그만 지우게 구현
    __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_CC1OF | TIM_FLAG_CC2OF | TIM_FLAG_CC3OF);
}

// [콜백 함수 - STM32가 자동으로 부르는 함수]
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM4) {
        // 여기서도 ID로 매핑해서 호출하면 더 완벽하지만,
        // 일단은 extern 객체를 불러서 처리하는 게 가장 쉬움
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
             Sonar_ISR_Process(&sonarRight);
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
             Sonar_ISR_Process(&sonarLeft);
        else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)
             Sonar_ISR_Process(&sonarCenter);
    }
}

void BSP_Delay_us(uint32_t us)
{
	__HAL_TIM_SET_COUNTER(&htim11, 0);
	while((__HAL_TIM_GET_COUNTER(&htim11)) < us);
}
