#include <Servo.h>

// Arduino pin assignment
#define PIN_IR    0
#define PIN_LED   9
#define PIN_SERVO 10

// configurable parameters
#define _DUTY_MIN 550
#define _DUTY_NEU 1500
#define _DUTY_MAX 2450

#define _DIST_MIN 100.0
#define _DIST_MAX 250.0

#define EMA_ALPHA 0.35
#define LOOP_INTERVAL 40

Servo myservo;
unsigned long last_loop_time = 0;

float dist_prev = 175.0;
float dist_ema = 175.0;

void setup()
{
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  myservo.attach(PIN_SERVO, _DUTY_MIN, _DUTY_MAX);
  myservo.writeMicroseconds(_DUTY_NEU);

  Serial.begin(1000000);
}

void loop()
{
  unsigned long time_curr = millis();
  int duty;
  float a_value, dist_raw, dist_filtered;

  if (time_curr - last_loop_time < LOOP_INTERVAL)
    return;
  last_loop_time += LOOP_INTERVAL;

  // IR Sensor
  a_value = analogRead(PIN_IR);

  // Distance Conversion (mm)
  if (a_value > 9.0) {
    dist_raw = (6762.0 / (a_value - 9.0) - 4.0) * 10.0 - 60.0;
  }
  else {
    dist_raw = -1.0;
  }

  // LED Control
  bool in_range = (dist_raw >= _DIST_MIN && dist_raw <= _DIST_MAX);
  digitalWrite(PIN_LED, in_range ? LOW : HIGH);

  // Range Filter
  if (a_value <= 9.0) {
    dist_filtered = dist_prev;
  }
  else if (dist_raw < _DIST_MIN) {
    dist_filtered = _DIST_MIN;
  }
  else if (dist_raw > _DIST_MAX) {
    dist_filtered = _DIST_MAX;
  }
  else {
    dist_filtered = dist_raw;
  }

  dist_prev = dist_filtered;

  // EMA Filter
  dist_ema = EMA_ALPHA * dist_filtered
           + (1.0 - EMA_ALPHA) * dist_ema;

  // Linear Mapping
  duty = _DUTY_MIN
       + (dist_ema - _DIST_MIN)
       * (_DUTY_MAX - _DUTY_MIN)
       / (_DIST_MAX - _DIST_MIN);

  duty = constrain(duty, _DUTY_MIN, _DUTY_MAX);

  myservo.writeMicroseconds(duty);

  // Serial Plotter
  Serial.print("_DUTY_MIN:");  Serial.print(_DUTY_MIN);
  Serial.print("_DIST_MIN:");  Serial.print(_DIST_MIN);
  Serial.print(",IR:");        Serial.print(a_value);
  Serial.print(",dist_raw:");  Serial.print(dist_raw);
  Serial.print(",ema:");       Serial.print(dist_ema);
  Serial.print(",servo:");     Serial.print(duty);
  Serial.print(",_DIST_MAX:"); Serial.print(_DIST_MAX);
  Serial.print(",_DUTY_MAX:"); Serial.print(_DUTY_MAX);
  Serial.println("");
}