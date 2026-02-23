/*
 * bsp_gpio.h
 *
 *  Created on: Dec 8, 2025
 *      Author: user
 */

#ifndef INC_BSP_GPIO_H_
#define INC_BSP_GPIO_H_

#include <stdbool.h> // 'bool', 'true', 'false'를 쓰기 위해
#include <stdint.h>  // 'uint8_t', 'uint16_t' 등을 쓰기 위해

// 1. 쓸 핀들에 '별명'을 붙이기
typedef enum {
    BSP_PIN_TRIG_LEFT,    // PC8
    BSP_PIN_TRIG_RIGHT,   // PC5
    BSP_PIN_TRIG_CENTER,  // PC6
	BSP_PIN_BUTTON_USER,  // B1
    // ... 필요한 거 추가
} bsp_pin_t;

// 2. 함수 선언 (STM32 몰라도 쓸 수 있게!)
void BSP_GPIO_Write(bsp_pin_t pin, bool state);
bool BSP_GPIO_Read(bsp_pin_t pin);
void BSP_GPIO_Toggle(bsp_pin_t pin);

#endif /* INC_BSP_GPIO_H_ */
