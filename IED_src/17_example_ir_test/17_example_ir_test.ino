// Arduino pin assignment

#define PIN_IR    0         // IR sensor at Pin A0

void setup()
{
  Serial.begin(57600);
}

void loop()
{
  int a_value = analogRead(PIN_IR);

  Serial.print("ADCRead:"); 
  Serial.println(a_value);
  // Serial.print(" = ");
  // Serial.print((a_value / 1024.0) * 5.0);
  // Serial.println(" Volt");
  delay(100);
}
