// 블루투스 없이 모터만 테스트하는 코드
// 전진 → 정지 → 후진 → 정지 → 좌회전 → 정지 → 우회전 → 정지 를 반복

const int A_IA = 5, A_IB = 7;   // 왼쪽 모터 (IA: PWM 속도, IB: 방향)
const int B_IA = 6, B_IB = 8;   // 오른쪽 모터

const int SPEED = 200;          // 속도 (0~255)
const int MOVE_MS = 2000;       // 각 동작 시간 (2초)
const int STOP_MS = 1000;       // 동작 사이 정지 시간 (1초)

void setup() {
  pinMode(A_IA, OUTPUT); pinMode(A_IB, OUTPUT);
  pinMode(B_IA, OUTPUT); pinMode(B_IB, OUTPUT);
  Serial.begin(9600);
  drive(0, 0);
  delay(2000);                  // 전원 켜고 2초 후 시작 (차 내려놓을 시간)
}

void loop() {
  Serial.println("전진");    drive(SPEED, SPEED);   delay(MOVE_MS);
  Serial.println("정지");    drive(0, 0);           delay(STOP_MS);

  Serial.println("후진");    drive(-SPEED, -SPEED); delay(MOVE_MS);
  Serial.println("정지");    drive(0, 0);           delay(STOP_MS);

  Serial.println("좌회전");  drive(-SPEED, SPEED);  delay(MOVE_MS);
  Serial.println("정지");    drive(0, 0);           delay(STOP_MS);

  Serial.println("우회전");  drive(SPEED, -SPEED);  delay(MOVE_MS);
  Serial.println("정지");    drive(0, 0);           delay(STOP_MS);
}

// L9110S 모터 1개 제어: v = -255(역회전) ~ 255(정회전)
void motor(int ia, int ib, int v) {
  v = constrain(v, -255, 255);
  if (v >= 0) {
    digitalWrite(ib, LOW);
    analogWrite(ia, v);
  } else {
    digitalWrite(ib, HIGH);
    analogWrite(ia, 255 + v);   // IB가 HIGH일 땐 듀티가 반전됨
  }
}

void drive(int left, int right) {
  motor(A_IA, A_IB, left);
  motor(B_IA, B_IB, right);
}
