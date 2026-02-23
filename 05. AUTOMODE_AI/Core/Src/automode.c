/*
 * automode.c
 * 4-Output RNN Model (PWM + Direction)
 */

#include "automode.h"
#include "robot_driver.h"
#include "network.h"
#include "network_data.h"
#include <stdio.h>
#include <string.h>

extern RobotCar_t myRobot;

// =========================================================
// 1. [AI 변수] RNN 설정
// =========================================================
#define RNN_WINDOW_SIZE 10
#define RNN_INPUT_DIM   3  // Front, Left, Right

static ai_handle network = AI_HANDLE_NULL;
AI_ALIGNED(32) static uint8_t pool[AI_NETWORK_DATA_ACTIVATIONS_SIZE];

// 입출력 버퍼
AI_ALIGNED(32) static ai_float ai_in_data[RNN_WINDOW_SIZE * RNN_INPUT_DIM];

// [핵심] 출력 버퍼가 4개로 늘어남 ([0]:L_PWM, [1]:R_PWM, [2]:L_DIR, [3]:R_DIR)
AI_ALIGNED(32) static ai_float ai_out_data[4];

// 시퀀스 히스토리 버퍼
static float sequence_buffer[RNN_WINDOW_SIZE][RNN_INPUT_DIM];

// =========================================================
// 2. [초기화]
// =========================================================
void AutoMode_Init(void)
{
    ai_error err;
    ai_handle w_handle = ai_network_data_weights_get();
    const ai_buffer w_buffer = AI_NETWORK_DATA_WEIGHTS(w_handle);

    err = ai_network_create(&network, &w_buffer);
    if (err.type != AI_ERROR_NONE) return;

    ai_network_params params = {
        .params = w_buffer,
        .activations = AI_NETWORK_DATA_ACTIVATIONS(pool)
    };

    if (!ai_network_init(network, &params)) {
        printf("RNN Init Fail!\n");
    }

    // 버퍼 초기화 (벽 없음 상태로 가정)
    for(int i=0; i<RNN_WINDOW_SIZE; i++) {
        sequence_buffer[i][0] = 1.0f;
        sequence_buffer[i][1] = 1.0f;
        sequence_buffer[i][2] = 1.0f;
    }
}

// =========================================================
// 3. [실행] RNN Inference
// =========================================================
AutoOutput_t AutoMode_Run(AutoInput_t input)
{
    AutoOutput_t output = {0};

    // --- A. 데이터 전처리 (동일) ---
    for(int i = 0; i < RNN_WINDOW_SIZE - 1; i++) {
        memcpy(sequence_buffer[i], sequence_buffer[i+1], sizeof(float) * RNN_INPUT_DIM);
    }
    sequence_buffer[RNN_WINDOW_SIZE-1][0] = (float)input.Dist_C / 200.0f;
    sequence_buffer[RNN_WINDOW_SIZE-1][1] = (float)input.Dist_L / 200.0f;
    sequence_buffer[RNN_WINDOW_SIZE-1][2] = (float)input.Dist_R / 200.0f;
    memcpy(ai_in_data, sequence_buffer, sizeof(ai_in_data));

    // --- B. AI 추론 (동일) ---
    ai_buffer* ai_input_buf = ai_network_inputs_get(network, NULL);
    ai_buffer* ai_output_buf = ai_network_outputs_get(network, NULL);

    if (!ai_input_buf || !ai_output_buf) return output;

    ai_input_buf[0].data = AI_HANDLE_PTR(ai_in_data);
    ai_output_buf[0].data = AI_HANDLE_PTR(ai_out_data); // 4-Output

    if (ai_network_run(network, ai_input_buf, ai_output_buf) != 1) return output;

    // --- C. 후처리 ---
	float raw_l_pwm = ai_out_data[0];
	float raw_r_pwm = ai_out_data[1];
	float raw_l_dir = ai_out_data[2];
	float raw_r_dir = ai_out_data[3];

	// 1. 방향 결정
	uint8_t l_go = (raw_l_dir > 0.5f) ? 1 : 0;
	uint8_t r_go = (raw_r_dir > 0.5f) ? 1 : 0;

	// 2. [부스터] 속도 계산
	int min_speed = 400;
	int max_speed = 1000;
	int range = max_speed - min_speed;

	int l_pwm = min_speed + (int)(raw_l_pwm * range);
	int r_pwm = min_speed + (int)(raw_r_pwm * range);

	if (l_pwm > 1000) l_pwm = 1000;
	if (r_pwm > 1000) r_pwm = 1000;

	// --- D. 결과 매핑 (GPIO 제어 삭제 -> 값만 전달) ---

	// [핵심] automode.c는 핀을 건드리지 않고, 값만 move.c에 전달합니다.
	output.Dir_L = l_go;
	output.Dir_R = r_go;

	// 멈출 때도 PWM 유지 (사용자 의도 반영)
	output.Speed_L = (uint16_t)l_pwm;
	output.Speed_R = (uint16_t)r_pwm;

	output.Action = (l_go || r_go) ? 1 : 0;

	return output;
}
