
#include <Servo.h>

// Arduino pin assignment
#define PIN_SERVO 10

// configurable parameters
#define _DUTY_MIN  550
#define _DUTY_NEU 1500
#define _DUTY_MAX 2450

// Loop intervals (msec)
#define _INTERVAL_SERVO 20UL
#define _INTERVAL_SERIAL 1000UL

// Experiment durations (msec)
#define _TIME_FAST  60000UL
#define _TIME_SLOW 300000UL

// Servo angular speed (degree/sec)
float _SERVO_SPEED = 3.0f;

// global variables
Servo myservo;

enum ExperimentState { FAST, SLOW, DONE };
ExperimentState state = FAST;

unsigned long stage_start_time = 0;
unsigned long last_servo_time = 0;
unsigned long last_serial_time = 0;

float angle_curr = 0.0f;

// Convert angle into servo pulse width
void writeServoAngle(float angle)
{
  angle = constrain(angle, 0.0f, 180.0f);

  int duty = (int)(_DUTY_MIN +
    (_DUTY_MAX - _DUTY_MIN) * (angle / 180.0f) + 0.5f);

  myservo.writeMicroseconds(duty);
  angle_curr = angle;
}

void setup()
{
  Serial.begin(57600);

  myservo.attach(PIN_SERVO, _DUTY_MIN, _DUTY_MAX);
  writeServoAngle(0.0f);

  // Wait until servo reaches the starting position
  delay(2000);

  stage_start_time = millis();
  last_servo_time = stage_start_time - _INTERVAL_SERVO;
  last_serial_time = stage_start_time;

  Serial.println("START: 3.0 deg/s, 0 -> 180 deg, 60 sec");
}

void loop()
{
  unsigned long time_curr = millis();
  unsigned long elapsed = time_curr - stage_start_time;

  // Experiment 1 -> Experiment 2
  if (state == FAST && elapsed >= _TIME_FAST) {
    writeServoAngle(180.0f);

    state = SLOW;
    _SERVO_SPEED = 0.3f;
    stage_start_time += _TIME_FAST;
    elapsed = time_curr - stage_start_time;

    Serial.println("START: 0.3 deg/s, 180 -> 90 deg, 300 sec");
  }

  // Finish Experiment 2
  if (state == SLOW && elapsed >= _TIME_SLOW) {
    writeServoAngle(90.0f);
    state = DONE;

    Serial.println("DONE: total experiment time = 360 sec");
  }

  if (state == DONE)
    return;

  // Servo update event
  if (time_curr - last_servo_time >= _INTERVAL_SERVO) {
    last_servo_time = time_curr;

    float elapsed_sec =
      (time_curr - stage_start_time) / 1000.0f;

    if (state == FAST)
      writeServoAngle(_SERVO_SPEED * elapsed_sec);
    else
      writeServoAngle(180.0f - _SERVO_SPEED * elapsed_sec);
  }

  // Serial output event
  if (time_curr - last_serial_time >= _INTERVAL_SERIAL) {
    last_serial_time = time_curr;

    Serial.print("stage:");
    Serial.print(state == FAST ? 1 : 2);

    Serial.print(",elapsed_sec:");
    Serial.print((time_curr - stage_start_time) / 1000.0f, 1);

    Serial.print(",speed_deg_s:");
    Serial.print(_SERVO_SPEED, 1);

    Serial.print(",angle_deg:");
    Serial.println(angle_curr, 2);
  }
}
