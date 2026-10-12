#include <Servo.h>

// Arduino pin assignment
#define PIN_LED   9
#define PIN_TRIG  12
#define PIN_ECHO  13
#define PIN_SERVO 10

// sonar
#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10

#define _DIST_MIN 180.0    // 18 cm
#define _DIST_MAX 360.0    // 36 cm

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

// EMA
#define _EMA_ALPHA 0.35

float dist_prev = _DIST_MIN;
float dist_ema = _DIST_MIN;

unsigned long last_sampling_time = 0;

Servo myservo;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_LED, HIGH);   // active-low LED OFF
  digitalWrite(PIN_TRIG, LOW);

  myservo.attach(PIN_SERVO);
  myservo.write(0);

  Serial.begin(57600);

  last_sampling_time = millis();
}

void loop() {
  if (millis() - last_sampling_time < INTERVAL)
    return;

  float dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);
  float dist_filtered;

  // -------------------------
  // Range Filter
  // -------------------------

  if (dist_raw == 0.0) {
    // measurement failure
    // keep previous valid value
    dist_filtered = dist_prev;
  }
  else if (dist_raw < _DIST_MIN) {
    // distance <= 18 cm
    dist_filtered = _DIST_MIN;
    dist_prev = dist_filtered;

    digitalWrite(PIN_LED, HIGH);
  }
  else if (dist_raw > _DIST_MAX) {
    // distance >= 36 cm
    dist_filtered = _DIST_MAX;
    dist_prev = dist_filtered;

    digitalWrite(PIN_LED, HIGH);
  }
  else {
    // distance is inside 18~36 cm
    dist_filtered = dist_raw;
    dist_prev = dist_raw;

    digitalWrite(PIN_LED, LOW);   // active-low LED ON
  }

  // -------------------------
  // EMA Filter
  // -------------------------

  dist_ema =
      _EMA_ALPHA * dist_filtered
      + (1.0 - _EMA_ALPHA) * dist_ema;

  // -------------------------
  // Distance -> Servo Angle
  // 180~360 mm -> 0~180 degree
  // -------------------------

  int servo_angle = map(
      (long)dist_ema,
      (long)_DIST_MIN,
      (long)_DIST_MAX,
      0,
      180
  );

  servo_angle = constrain(servo_angle, 0, 180);

  myservo.write(servo_angle);

  // -------------------------
  // Serial Plotter
  // -------------------------

  Serial.print("Min:");       Serial.print(_DIST_MIN);
  Serial.print(",Raw:");      Serial.print(dist_raw);
  Serial.print(",Filtered:"); Serial.print(dist_filtered);
  Serial.print(",ema:");      Serial.print(dist_ema);
  Serial.print(",Servo:");    Serial.print(servo_angle);
  Serial.print(",Max:");      Serial.print(_DIST_MAX);

  Serial.println();

  last_sampling_time += INTERVAL;
}


// ultrasonic sensor measurement
// return value: mm
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}