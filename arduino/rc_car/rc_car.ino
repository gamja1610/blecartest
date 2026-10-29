#include <SoftwareSerial.h>
SoftwareSerial bt(2, 3); // 블루투스: D2(RX) <- TXD, D3(TX) -> RXD

// ★ [우리 조 고유 설정] - 조마다 다르게!
const char BT_NAME[]  = "CAR01";  // 블루투스 이름 (CAR01 ~ CAR07)
const char PASSWORD[] = "1234";   // ★ 웹앱에 입력할 비밀번호 (숫자 4자리)

const int A_IA = 5, A_IB = 7;   // 왼쪽 모터 (IA:PWM 속도, IB:방향)
const int B_IA = 6, B_IB = 8;   // 오른쪽 모터 (IA:PWM 속도, IB:방향)
const unsigned long FAILSAFE_MS = 600;   // 0.6초 통신 안전장치
const unsigned long AUTH_MS = 30000;     // 30초 동안 명령 없으면 다시 잠금

int speed = 223;                // 기본 1단 속도 (223~255)
char cmd = 'S';                 // 현재 주행 상태 ('S':정지)
unsigned long lastCmd = 0;      // 마지막 수신 시각

// ★ 보안: 아두이노가 직접 비밀번호 검사
bool authed = false;            // 비밀번호 통과 여부
bool readingPw = false;         // 비밀번호 받는 중
String pw = "";
unsigned long lastLocked = 0;   // LOCKED 응답 간격 조절

void setup() {
  pinMode(A_IA, OUTPUT); pinMode(A_IB, OUTPUT);
  pinMode(B_IA, OUTPUT); pinMode(B_IB, OUTPUT);
  Serial.begin(9600);
  bt.begin(9600);
  drive(0, 0); // 시작 시 안전 정지 상태

  delay(300);
  bt.print(String("AT+NAME") + BT_NAME + "\r\n"); // 차량 이름 설정
  delay(300);
  while (bt.available()) Serial.write(bt.read()); // 모듈 응답 출력
}

void loop() {
  readBluetooth(); // ★ 비밀번호 검사 + 조종 명령 받기 (3/4)

  // ★ 자동 잠금: 모듈이 연결 끊김을 알려주지 않으므로 시간으로 잠금
  if (authed && millis() - lastCmd > AUTH_MS) authed = false;

  // 안전장치: 인증 안 됐거나 0.6초간 신호 끊기면 자동 정지
  if (!authed || (FAILSAFE_MS > 0 && millis() - lastCmd > FAILSAFE_MS)) cmd = 'S';
  applyManual(cmd);
}

// ★ 블루투스로 받은 글자 처리: 비밀번호 검사 → 조종 명령
void readBluetooth() {
  while (bt.available()) {
    char c = bt.read();
    // 비밀번호 받기: P로 시작해서 줄바꿈으로 끝남 (예: P1234)
    if (c == 'P' && !readingPw) { readingPw = true; pw = ""; continue; }
    if (readingPw) {
      if (c == '\n' || c == '\r') {
        readingPw = false;
        authed = (pw == PASSWORD);   // ★ 비밀번호 비교
        lastCmd = millis(); cmd = 'S';
        bt.print(authed ? "AUTH OK\n" : "AUTH FAIL\n");
      } else if (pw.length() < 16) pw += c;
      continue;
    }
    bool isGear = (c >= '1' && c <= '5');
    bool isDir = (c != 0 && strchr("FBLRGIHJS", c));
    if (!isGear && !isDir) continue;  // 그 밖의 글자는 무시
    if (!authed) {                    // ★ 인증 전에는 어떤 명령도 안 받음
      if (millis() - lastLocked > 1000) { bt.print("LOCKED\n"); lastLocked = millis(); }
      continue;
    }
    lastCmd = millis(); Serial.print(c);
    if (isGear) {
      speed = map(c - '0', 1, 5, 223, 255); // 1~5단 기어
      bt.print("GEAR "); bt.print(c); bt.print("\n");
    } else cmd = c;                          // 방향 명령
  }
}

// 방향 명령에 따라 바퀴 제어
void applyManual(char c) {
  int s = speed, h = speed / 2;
  switch (c) {
    case 'F': drive(s, s);   break; // 전진
    case 'B': drive(-s, -s); break; // 후진
    case 'L': drive(-s, s);  break; // 제자리 좌회전
    case 'R': drive(s, -s);  break; // 제자리 우회전
    case 'G': drive(h, s);   break; // 전진 완만 좌회전
    case 'I': drive(s, h);   break; // 전진 완만 우회전
    case 'H': drive(-h, -s); break; // 후진 완만 좌회전
    case 'J': drive(-s, -h); break; // 후진 완만 우회전
    default:  drive(0, 0);   break; // 정지 ('S')
  }
}

// L9110S 모터 1개: v = -255(역) ~ 255(정)
void motor(int ia, int ib, int v) {
  v = constrain(v, -255, 255); // 범위 밖 값 방지
  if (v >= 0) {
    digitalWrite(ib, LOW);     // 전진 방향
    analogWrite(ia, v);        // PWM 속도
  } else {
    digitalWrite(ib, HIGH);    // 후진 방향
    analogWrite(ia, 255 + v);  // 역방향 PWM
  }
}

void drive(int left, int right) {
  motor(A_IA, A_IB, left);     // 왼쪽 바퀴
  motor(B_IA, B_IB, right);    // 오른쪽 바퀴
}
