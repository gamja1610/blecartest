# blecartest — 하이스쿨 아두이노 블루투스 RC카 테스트

- **조종 웹앱**: https://gamja1610.github.io/blecartest/ (안드로이드 Chrome / 아이폰 Bluefy)
- 폰 ↔ RC카 통신은 블루투스(BLE)로만 합니다. 인터넷은 페이지를 여는 순간에만 사용합니다.

## 파일

| 파일 | 내용 |
|---|---|
| `index.html` | RC카 조종 웹앱 (비번 인증, 핸들·페달·기어 1~5단) |
| `arduino/blecartest/blecartest.ino` | RC카 코드 (BLE 비번 인증 + 모터 제어 + 초음파 정지) |
| `arduino/motor_test/motor_test.ino` | 블루투스 없이 모터만 확인 (전진→후진→좌→우 반복) |

## 차량 설정 (`blecartest.ino` 맨 위)

| 항목 | 값 |
|---|---|
| `BT_NAME` | 블루투스 목록에 뜨는 이름 (테스트 차: `speedstarX`) |
| `PASSWORD` | 웹앱에 입력할 숫자 4자리 (테스트 차: `1234`) |

## 배선

| 부품 | 핀 | 우노 |
|---|---|---|
| BLE (HM-10/BT05) | VCC / GND / TXD / RXD | 5V / GND / D2 / D3 |
| L9110S | A-IA / A-IB / B-IA / B-IB | D5 / D7 / D6 / D8 |
| L9110S | VCC / GND | AA(+) 외부 전원 / AA(−) + 우노 GND |
| 초음파 (선택) | Vcc / Trig / Echo / Gnd | 5V / D10 / D11 / GND |

## 사용 순서

1. `blecartest.ino` 업로드 (폰과 연결 안 된 상태에서 켜야 이름이 적용됨)
2. 웹앱 열기 → 비번 4자리 입력 → `차량 연결` → 목록에서 차 이름 선택
3. 목록에 안 뜨면 `전체` 버튼을 켜고 다시 연결

## 통신 규칙

| 폰 → 아두이노 | 의미 |
|---|---|
| `P`+비번+줄바꿈 | 인증 → `AUTH OK` / `AUTH FAIL` |
| `F B L R G I H J` / `S` | 방향 / 정지 (인증 후에만, 0.15초마다 반복 전송) |
| `1`~`5` | 기어 → `GEAR n` |

- 인증 전 명령은 무시(`LOCKED`), 30초 방치 시 다시 잠금 → 웹앱이 자동 재인증
- 0.6초 동안 명령이 없으면 자동 정지
