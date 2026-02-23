/* ultrasonic.h */
#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#include "bsp_gpio.h"
#include "bsp_timer.h"

#define MEDIAN_WIN           3u

// [설계도] 소나(Sonar) 센서 객체 정의
typedef struct {
    // 1. 하드웨어 설정 (추상화된 ID만 사용!)
	bsp_timer_t       timer_id;
    bsp_pin_t         trig_pin_id;

    // 2. 측정 데이터
    volatile uint8_t  state;
    volatile uint32_t ic_start;
    volatile uint32_t ic_end;
    volatile float    distance;

    // 3. 필터용 버퍼
    uint16_t med_buf[MEDIAN_WIN];
    uint8_t  med_head;
    uint8_t  med_count;

} Sonar_t;

// [함수 선언]
void Sonar_Init(void);
void Sonar_Trigger(Sonar_t *sonar);
void Sonar_ISR_Process(Sonar_t *sonar);
float Sonar_Get_Distance(Sonar_t *sonar);

extern Sonar_t sonarLeft;
extern Sonar_t sonarRight;
extern Sonar_t sonarCenter;

#endif /* ULTRASONIC_H_ */
