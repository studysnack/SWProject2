#define PIN 7

void setup() {
  pinMode(PIN, OUTPUT);
}

void loop() {
  // LED ON
  digitalWrite(PIN, LOW);

  delay(1000);

  for(int i = 0; i < 5; i++){
    //LED OFF
    digitalWrite(PIN, HIGH);

    delay(100);

    //LED ON
    digitalWrite(PIN, LOW);

    delay(100);
  }

  //LED OFF
  digitalWrite(PIN, HIGH);

  while(1){}
}
