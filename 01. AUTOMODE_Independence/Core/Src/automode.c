/*
 * automode.c — Clean & Readable Autonomous Drive
 */

#include "automode.h"
#include <stdbool.h>
#include <stdint.h>

// =====================================================
// 1. [튜닝 파라미터] 로봇의 성격 (상수로 관리)
// =====================================================
// [거리 제어]
#define TARGET_DIST_CM      15      // 목표 정지 거리
#define RECOVERY_DIST_CM    80      // 회피 후 이 정도 뚫리면 다시 주행

// [속도 제어]
#define SPEED_MIN           300     // 모터 구동 최소 PWM
#define SPEED_MAX           600     // 최대 속도
#define SPEED_AVOID         350     // 회피(후진/회전) 시 속도
#define KP_NUM              15      // P-Gain 분자
#define KP_DEN              1       // P-Gain 분모

// [시간 제어]
#define TIME_BACK_MS        500     // 후진 시간
#define TIME_TURN_MS        400     // 회전 시간

// =====================================================
// 2. [내부 상태 정의]
// =====================================================
typedef enum {
    STATE_DRIVE,       // P-제어 주행
    STATE_AVOID_BACK,  // 장애물 감지 -> 후진
    STATE_AVOID_TURN   // 후진 완료 -> 회전
} State_t;

// =====================================================
// 3. [도우미 함수] 복잡한 계산은 여기서 숨김 (Helper)
// =====================================================
static int Clip(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// P-제어 계산기: 거리에 따라 적절한 속도를 계산해준다.
static int Calculate_CruiseSpeed(int current_dist)
{
    // 1. 오차 계산
    int error = current_dist - TARGET_DIST_CM;

    // 2. P-제어 공식 (Gain 적용)
    int p_out = (error * KP_NUM) / KP_DEN;

    // 3. 기본 속도 더하기 (Deadzone 보정)
    int final_speed = p_out + SPEED_MIN;

    // 4. 범위 제한 (0 ~ MAX)
    return Clip(final_speed, SPEED_MIN, SPEED_MAX);
}

// =====================================================
// 4. [메인 뇌 함수] 흐름이 한눈에 보여야 한다!
// =====================================================
AutoOutput_t AutoMode_Run(AutoInput_t input)
{
    // 기본값: 정지
    AutoOutput_t output = { .Action = ACTION_STOP, .Speed_L = 0, .Speed_R = 0 };

    // [상태 저장소] (static으로 유지)
    static State_t  s_state    = STATE_DRIVE; // 이제 enum을 직접 쓴다!
    static uint32_t s_deadline = 0;           // 목표 시간
    static int      s_turn_dir = 1;           // 1:Right, -1:Left

    switch (s_state)
    {
        // -------------------------------------------------
        case STATE_DRIVE:
        // -------------------------------------------------
            // [판단 1] 너무 가까운가? -> 도망가자!
            if (input.Dist_C <= TARGET_DIST_CM) {
                s_state = STATE_AVOID_BACK;
                s_deadline = input.Current_Time_ms + TIME_BACK_MS;
            }
            // [판단 2] 아니면 그냥 달리자 (P-제어)
            else {
                int speed = Calculate_CruiseSpeed(input.Dist_C);

                output.Action  = ACTION_FORWARD;
                output.Speed_L = (uint16_t)speed;
                output.Speed_R = (uint16_t)speed;
            }
            break;

        // -------------------------------------------------
        case STATE_AVOID_BACK:
        // -------------------------------------------------
            output.Action  = ACTION_BACKWARD;
            output.Speed_L = SPEED_AVOID;
            output.Speed_R = SPEED_AVOID;

            // 시간이 다 됐으면 -> 회전으로 넘어감
            if (input.Current_Time_ms >= s_deadline) {
                s_state = STATE_AVOID_TURN;
                s_deadline = input.Current_Time_ms + TIME_TURN_MS;

                // 넓은 쪽으로 돌자
                s_turn_dir = (input.Dist_L > input.Dist_R) ? -1 : 1;
            }
            break;

        // -------------------------------------------------
        case STATE_AVOID_TURN:
        // -------------------------------------------------
            output.Action  = (s_turn_dir > 0) ? ACTION_PIVOT_RIGHT : ACTION_PIVOT_LEFT;
            output.Speed_L = SPEED_AVOID;
            output.Speed_R = SPEED_AVOID;

            // 시간이 다 됐거나, 앞이 뻥 뚫렸으면 -> 다시 주행
            if (input.Current_Time_ms >= s_deadline || input.Dist_C > RECOVERY_DIST_CM) {
                s_state = STATE_DRIVE;
            }
            break;
    }

    return output;
}
