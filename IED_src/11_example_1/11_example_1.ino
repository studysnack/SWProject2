#include <Servo.h>

// Arduino pin assignment
#define PIN_LED   9   // LED active-low
#define PIN_TRIG  12  // sonar sensor TRIGGER
#define PIN_ECHO  13  // sonar sensor ECHO
#define PIN_SERVO 10  // servo motor

// configurable parameters for sonar
#define SND_VEL 346.0      // sound velocity at 24 celsius degree (unit: m/sec)
#define INTERVAL 25        // sampling interval (unit: msec)
#define PULSE_DURATION 10  // ultra-sound pulse duration (unit: usec)
#define _DIST_MIN 180.0    // minimum distance to be measured (unit: mm)
#define _DIST_MAX 360.0    // maximum distance to be measured (unit: mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

// EMA
#define _EMA_ALPHA 0.3

// Target Distance
#define _TARGET_LOW  250.0
#define _TARGET_HIGH 290.0

// duty duration for myservo.writeMicroseconds()
// NEEDS TUNING (servo by servo)
#define _DUTY_MIN 550  // servo 0 degree
#define _DUTY_NEU 1500  // servo 90 degree
#define _DUTY_MAX 2450  // servo 180 degree

// global variables
float dist_ema;
float dist_prev = _DIST_MAX;

unsigned long last_sampling_time = 0;

Servo myservo;

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_LED, HIGH);   // active-low LED OFF
  digitalWrite(PIN_TRIG, LOW);

  // initialize servo
  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_NEU);

  digitalWrite(PIN_LED, HIGH);

  // initialize USS / EMA variables
  dist_prev = _DIST_MIN;
  dist_ema = _DIST_MIN;

  // initialize sampling time
  last_sampling_time = millis();

  // initialize serial port
  Serial.begin(57600);
}

void loop() {
  float dist_raw;
  float dist_filtered;

  // wait until next sampling time
  if (millis() - last_sampling_time < INTERVAL)
    return;

  // get a distance reading from the USS
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  // range filter
  if ((dist_raw == 0.0) || (dist_raw > _DIST_MAX)) {
    dist_filtered = dist_prev;
  }
  else if (dist_raw < _DIST_MIN) {
    dist_filtered = dist_prev;
  }
  else {
    dist_filtered = dist_raw;
    dist_prev = dist_raw;
  }

  // EMA filter
  dist_ema =
      _EMA_ALPHA * dist_filtered
      + (1.0 - _EMA_ALPHA) * dist_ema;

  // adjust servo position according to distance
  if (dist_ema < _TARGET_LOW) {
    // too close
    myservo.writeMicroseconds(_DUTY_MIN);
    digitalWrite(PIN_LED, HIGH);
  }
  else if (dist_ema > _TARGET_HIGH) {
    // too far
    myservo.writeMicroseconds(_DUTY_MAX);
    digitalWrite(PIN_LED, HIGH);
  }
  else {
    // target range
    myservo.writeMicroseconds(_DUTY_NEU);
    digitalWrite(PIN_LED, LOW);
  }

  // output values to Serial Plotter / Serial Monitor
  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",Low:");
  Serial.print(_TARGET_LOW);

  Serial.print(",dist:");
  Serial.print(dist_ema);

  Serial.print(",Servo:");
  Serial.print(myservo.read());

  Serial.print(",High:");
  Serial.print(_TARGET_HIGH);

  Serial.print(",Max:");
  Serial.print(_DIST_MAX);

  Serial.println();

  // update last sampling time
  last_sampling_time += INTERVAL;
}


// get a distance reading from USS
// return value: millimeter
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}