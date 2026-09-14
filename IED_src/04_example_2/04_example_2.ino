void setup() {
  Serial.begin(115200);
  while(!Serial){
    ;
  }
}

void loop() {
  Serial.println("Hello world!");
  delay(1000);
}
