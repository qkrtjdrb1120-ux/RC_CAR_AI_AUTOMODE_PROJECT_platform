import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sqlalchemy import create_engine
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Input, LSTM, Dense, Dropout

# --- [파트 1: 데이터 로드 및 전처리] ---
def load_and_preprocess():
    # Tailscale IP를 통한 DB 연결
    engine = create_engine("mysql+pymysql://root:1234@100.65.81.44/rc_car_project")
    query = "SELECT * FROM driving_log WHERE member_name = 'LEJ' AND run_id = 2"
    df = pd.read_sql(query, engine)

    # ms -> s 단위 변환 (사용자님 제안)
    df['timestamp_s'] = df['timestamp_ms'] / 1000.0

    # 0~1 사이 정규화 (Scaling)
    max_dist = 200.0
    df['front_norm'] = df['front_dist'] / max_dist
    df['left_norm'] = df['left_dist'] / max_dist
    df['right_norm'] = df['right_dist'] / max_dist
    df['l_pwm_norm'] = df['l_pwm'] / 1000.0
    df['r_pwm_norm'] = df['r_pwm'] / 1000.0

    return df


# --- [파트 2: RNN용 시퀀스 데이터 생성] ---
def prepare_sequences(df, window_size=10):
    X, y = [], []
    features = df[['front_norm', 'left_norm', 'right_norm']].values
    targets = df[['l_pwm_norm', 'r_pwm_norm']].values

    for i in range(len(df) - window_size):
        X.append(features[i: i + window_size])
        y.append(targets[i + window_size])

    return np.array(X), np.array(y)

# --- [파트 3: 메인 실행 로직] ---
# 1. 함수를 호출하여 데이터를 가져옵니다.
df_processed = load_and_preprocess()

# 2. RNN 학습을 위한 3차원 데이터로 변환
X_train, y_train = prepare_sequences(df_processed, window_size=10)

# 3. 모델 정의 (LSTM)
model = Sequential([
    # 1. 여기서 배치 크기(1)와 입력 형태(10, 3)를 명확히 정해줍니다.
    Input(batch_shape=(1, 10, 3)),

    # 2. LSTM 레이어에서는 shape 관련 인자를 뺍니다.
    LSTM(64),

    Dropout(0.2),
    Dense(32, activation='relu'),
    Dense(2, activation='sigmoid')
])

model.compile(optimizer='adam', loss='mse')

# 4. 실제 학습 시작
print("학습을 시작합니다...")
# [중요!] 모델의 배치 크기를 1로 고정했으므로, 학습할 때도 batch_size=1로 맞춰야 합니다.
# 데이터가 500개 정도로 적어서 속도 차이는 거의 없습니다.
model.fit(X_train, y_train, epochs=50, batch_size=1, validation_split=0.2)
# 학습이 끝난 후 모델을 .keras 포맷으로 임시 저장
model.save("temp_model.keras")

# 5. 학습된 모델로 예측 수행
# X_train 데이터를 넣어 AI가 판단한 PWM 결과값을 받아옵니다.
predictions = model.predict(X_train)

# 6. 결과 시각화
plt.figure(figsize=(12, 6))

# 왼쪽 모터(L_PWM) 비교
plt.subplot(2, 1, 1)
plt.plot(y_train[:, 0], label='Actual L_PWM', color='blue', alpha=0.5)
plt.plot(predictions[:, 0], label='AI Predicted L_PWM', color='red', linestyle='--')
plt.title('AI Prediction vs Actual Data (Left Motor)')
plt.legend()

# 오른쪽 모터(R_PWM) 비교
plt.subplot(2, 1, 2)
plt.plot(y_train[:, 1], label='Actual R_PWM', color='green', alpha=0.5)
plt.plot(predictions[:, 1], label='AI Predicted R_PWM', color='orange', linestyle='--')
plt.title('AI Prediction vs Actual Data (Right Motor)')
plt.legend()

plt.tight_layout()
plt.show()
