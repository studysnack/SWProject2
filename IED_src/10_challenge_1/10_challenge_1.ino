#include <Servo.h>
#include <cmath>

// Arduino pin assignment
#define PIN_SERVO 10
#define PIN_TRIG  12
#define PIN_ECHO  13

// configurable parameters
#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10
#define DETECT_DISTANCE 150

#define TIMEOUT ((unsigned long)(INTERVAL * 1000UL / 2))
#define SCALE (0.001 * 0.5 * SND_VEL)

#define CURVE_SIGMOID 0
#define CURVE_GAUSSIAN 1

#define CURVE_TYPE CURVE_SIGMOID

Servo myServo;

unsigned long last_sampling_time = 0;

unsigned long MOVING_TIME = 1000; // moving time is 1 second
unsigned long moveStartTime;

int startAngle = 180;
int stopAngle = 180;
int currentAngle = 180;

bool isInside = false;
bool isMoving = false;


// function prototypes
float USS_measure(int TRIG, int ECHO);
double sigmoid(double x);
double gaussian(double x);
long curveAngle(unsigned long progress);
void moveServo(int angle);
void updateServo();


void setup() {
  myServo.attach(PIN_SERVO);
  myServo.write(startAngle); // Set position

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  Serial.begin(57600);
}


void loop() {
  updateServo();

  // wait until next sampling time
  if (millis() - last_sampling_time < INTERVAL)
    return;

  last_sampling_time += INTERVAL;

  // get a distance reading from the USS
  float dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);

  Serial.print("Distance:");
  Serial.print(dist_raw);

  Serial.print(",Boundary:");
  Serial.print(DETECT_DISTANCE);

  Serial.println();

  if (dist_raw < 0)
    return;

  if (dist_raw <= DETECT_DISTANCE && !isInside) {
    Serial.println("in");

    isInside = true;
    moveServo(90);
  }
  else if (dist_raw > DETECT_DISTANCE && isInside) {
    Serial.println("out");

    isInside = false;
    moveServo(180);
  }
}


// get a distance reading from USS
float USS_measure(int TRIG, int ECHO) {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  unsigned long duration = pulseIn(ECHO, HIGH, TIMEOUT);

  if (duration == 0)
    return -1.0;

  return duration * SCALE;
}


double sigmoid(double x) {
  return 1.0 / (1.0 + exp(-x));
}


double gaussian(double x) {
  return exp(-(x * x));
}


// calculate angle using selected curve
long curveAngle(unsigned long progress) {
  double x = (double)progress / MOVING_TIME;

#if CURVE_TYPE == CURVE_SIGMOID

  long value = (long)(sigmoid(x) * 1000);

  return map(
    value,
    (long)(sigmoid(0.0) * 1000),
    (long)(sigmoid(1.0) * 1000),
    startAngle,
    stopAngle
  );

#elif CURVE_TYPE == CURVE_GAUSSIAN

  long value = (long)(gaussian(x) * 1000);

  return map(
    value,
    (long)(gaussian(0.0) * 1000),
    (long)(gaussian(1.0) * 1000),
    startAngle,
    stopAngle
  );

#endif
}


// start moving
void moveServo(int angle) {
  startAngle = currentAngle;
  stopAngle = angle;

  moveStartTime = millis(); // start moving
  isMoving = true;
}


void updateServo() {
  if (!isMoving)
    return;

  unsigned long progress = millis() - moveStartTime;

  if (progress <= MOVING_TIME) {
    // while moving
    long angle = curveAngle(progress);

    currentAngle = angle;
    myServo.write(angle);
  }
  else {
    // movement finished
    currentAngle = stopAngle;
    myServo.write(stopAngle);

    isMoving = false;
  }
}