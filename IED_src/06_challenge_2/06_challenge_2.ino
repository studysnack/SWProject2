unsigned long pwm_period;
int pwm_duty;

void setup() {
  pinMode(7, OUTPUT);

  set_period(100);   // 100 ~ 10000 us
  set_duty(0);
}

void loop() {
  static unsigned long start_time = micros();

  unsigned long elapsed = micros() - start_time;

  // 1초가 지나면 새로운 triangle 시작
  if (elapsed >= 1000000UL) {
    start_time += 1000000UL;
    elapsed = micros() - start_time;
  }

  // 0 ~ 0.5초 : 0% -> 100%
  if (elapsed <= 500000UL) {
    set_duty((elapsed * 100UL) / 500000UL);
  }

  // 0.5 ~ 1초 : 100% -> 0%
  else {
    set_duty(((1000000UL - elapsed) * 100UL) / 500000UL);
  }

  pwm();
}

void set_period(int period) {
  pwm_period = period;
}

void set_duty(int duty) {
  pwm_duty = duty;
}

void pwm() {
    int on_time = pwm_period * pwm_duty / 100;
    int off_time = pwm_period - on_time;

    digitalWrite(7, LOW);       // LED ON
    delayMicroseconds(on_time);

    digitalWrite(7, HIGH);      // LED OFF
    delayMicroseconds(off_time);
}