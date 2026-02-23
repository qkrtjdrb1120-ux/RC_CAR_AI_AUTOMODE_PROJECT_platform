/*
 * bsp_gpio.c
 *
 *  Created on: Dec 8, 2025
 *      Author: user
 */

/* Core/Src/BSP/bsp_gpio.c */
#include "bsp_gpio.h"
#include "main.h"  // <--- 중요! 여기에 ioc에서 설정한 핀 이름들이 들어있음
#include "tim.h"

// LED나 초음파 Trigger 처럼 "내가 신호를 보내는 핀"들을 위한 것
void BSP_GPIO_Write(bsp_pin_t pin, bool state)
{
    // 1. bool(true/false)을 STM32가 아는 언어(SET/RESET)로 변환
    GPIO_PinState pin_state = (state) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    // 2. "별명(pin)"을 보고 "실제 주소"를 찾아서 실행 (Switch-case)
    switch (pin)
    {
        // [CASE 1] 왼쪽 초음파 발사!
        case BSP_PIN_TRIG_LEFT:
        	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, pin_state);
            break;

        // [CASE 2] 오른쪽 초음파 발사!
        case BSP_PIN_TRIG_RIGHT:
        	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, pin_state);
            break;

        // [CASE 3] 중앙 초음파 발사!
        case BSP_PIN_TRIG_CENTER:
        	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, pin_state);
            break;

        default:
            // 없는 핀 번호가 들어오면 무시 (혹은 에러 처리)
            break;
    }
}

// 버튼이나 리미트 스위치, 혹은 초음파 Echo(폴링 방식일 때) 처럼 "외부 신호를 읽어오는 핀"들을 위한 것
bool BSP_GPIO_Read(bsp_pin_t pin)
{
    // 기본값은 false(0)로 둠
    GPIO_PinState state = GPIO_PIN_RESET;

    switch (pin)
    {
        // [CASE 1] 파란색 유저 버튼 (B1)
        // (보통 PC13 핀에 연결되어 있음. main.h의 B1_GPIO_Port 확인!)
        case BSP_PIN_BUTTON_USER:
             // 버튼이 눌렸는지 확인
            state = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin);
            break;
             // 나중에 라인 센서 같은 게 추가되면 여기에 case 추가!
        default:
             // 없는 핀 번호가 들어오면 무시 (혹은 에러 처리)
            break;
    }

    // STM32는 눌리면 SET(1)일 수도 있고 RESET(0)일 수도 있는데,
    // 보통 SET(1)을 true로 리턴하면 편함.
    return (state == GPIO_PIN_SET);
}


// LED를 깜빡여서 "나 살아있어요" 하고 생존 신고할 때
void BSP_GPIO_Toggle(bsp_pin_t pin)
{
    switch (pin)
    {
//        case BSP_PIN_BUZZER:
//            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
//            break;
            // 트리거 핀은 토글할 일이 없으니 안 넣어도 됨
        default:
            // 없는 핀 번호가 들어오면 무시 (혹은 에러 처리)
            break;
    }
}
