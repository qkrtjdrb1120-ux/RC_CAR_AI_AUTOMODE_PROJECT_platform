import tensorflow as tf

# 1. 저장된 모델 불러오기
model = tf.keras.models.load_model("temp_model.keras")

# 2. TFLite 변환기 생성
converter = tf.lite.TFLiteConverter.from_keras_model(model)

# [핵심 수정] STM32(TFLite Micro) 호환성을 위한 설정
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS]

# [핵심 추가] LSTM 연산을 TFLite 표준 연산으로 풀어서(Unroll) 변환
converter._experimental_lower_tensor_list_ops = True

# 3. 변환 및 파일 저장 (try-except로 안전하게 처리)
try:
    print("STM32용 모델 변환 중...")
    tflite_model = converter.convert()  # 여기서 한 번만 실행합니다.

    with open("rc_car_rnn_model_pure.tflite", "wb") as f:
        f.write(tflite_model)
    print("성공! STM32 최적화 모델(rc_car_rnn_model_pure.tflite) 저장 완료!")

except Exception as e:
    print(f"변환 실패: {e}")
    print("힌트: 모델 구조에 TFLite Micro가 지원하지 않는 특수 레이어가 있는지 확인해 보세요.")
