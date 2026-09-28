// 아두이노 핀 할당
#define PIN_LED 9
#define PIN_TRIG 12   // 초음파 센서 TRIGGER
#define PIN_ECHO 13   // 초음파 센서 ECHO

// 설정 가능한 매개변수
#define SND_VEL 346.0     // 섭씨 24도에서의 음속 (단위: m/sec)
#define INTERVAL 100      // 샘플링 간격 (단위: msec)
#define PULSE_DURATION 10 // 초음파 펄스 지속 시간 (단위: usec)
#define _DIST_MIN 100.0   // 측정할 최소 거리 (단위: mm)
#define _DIST_MAX 300.0   // 측정할 최대 거리 (단위: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // 최대 ECHO 대기 시간 (단위: usec)
#define SCALE (0.001 * 0.5 * SND_VEL)     // 측정 시간을 거리로 변환하기 위한 계수

void setup() {
  // GPIO 핀 초기화
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);    // 초음파 센서 TRIGGER
  pinMode(PIN_ECHO, INPUT);     // 초음파 센서 ECHO
  digitalWrite(PIN_TRIG, LOW);  // 초음파 센서 비활성화
  
  // 시리얼 포트 초기화
  Serial.begin(57600);
}

void loop() {
  float distance = USS_measure(PIN_TRIG, PIN_ECHO); // 거리 측정

  if ((distance == 0.0) || (distance > _DIST_MAX)) {
    distance = _DIST_MAX + 10.0;      // 최대 범위보다 큰 값으로 설정
    digitalWrite(PIN_LED, 1);         // LED 끄기
  } else if (distance < _DIST_MIN) {
    distance = _DIST_MIN - 10.0;      // 최소 범위보다 작은 값으로 설정
    digitalWrite(PIN_LED, 1);         // LED 끄기
  } else {                            // 원하는 측정 범위 내에 있는 경우
    digitalWrite(PIN_LED, 0);         // LED 켜기
  }

  // 측정한 거리를 시리얼 포트로 출력
  Serial.print("Min:");        Serial.print(_DIST_MIN);
  Serial.print(",distance:");  Serial.print(distance);
  Serial.print(",Max:");       Serial.print(_DIST_MAX);
  Serial.println("");
  
  // 다른 작업을 수행하는 부분
  delay(50); // 다른 작업을 수행하는 데 50ms가 걸린다고 가정
  
  // 다음 샘플링 시간까지 대기
  delay(INTERVAL);
}

// 초음파 센서에서 거리를 측정한다. 반환값의 단위는 mm이다.
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);
  
  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE; // 단위: mm

  // 펄스 지속 시간을 거리로 변환하는 예시 (대상 거리 = 17.3m)
  // - pulseIn(ECHO, HIGH, timeout)은 마이크로초 단위의 시간을 반환한다. (음파의 왕복 시간)
  // - 편도 거리 = (pulseIn() / 1,000,000) * SND_VEL / 2 (미터 단위)
  //   mm 단위로 변환하려면 * 1,000이 필요 ==> SCALE = 0.001 * 0.5 * SND_VEL
  //
  // - 예: pulseIn()이 100,000이면 (= 0.1초, 왕복 거리 34.6m)
  //        = 100,000 micro*sec * 0.001 milli/micro * 0.5 * 346 meter/sec
  //        = 100,000 * 0.001 * 0.5 * 346
  //        = 17,300 mm  ==> 17.3m
}