void setup() {
  // Set pin 40 to be an input pin and pin 19 to be output
  pinMode(40, INPUT);
  pinMode(19, OUTPUT);
}

void loop() {
  // read the button input via pin 40.
  // if value is "high" turn on LED
  // else LED is off
  if (digitalRead(40)) {
    digitalWrite(19, HIGH);
    Serial.print(digitalRead(40));
  } else {
    digitalWrite(19, LOW);
  }


}
