void setup() {
  // set pin 19 and pin 21 to be used for output
  pinMode(19, OUTPUT);
  pinMode(21, OUTPUT);

}

void loop() {
  digitalWrite(19, HIGH);   // sets pin 19 to "high" output
  digitalWrite(21, LOW);    // sets pin 21 to "low" output
  delay(1000);              // 1 second delay
  digitalWrite(19, LOW);    // sets pin 19 to "low" output
  digitalWrite(21, HIGH);   // sets pin 21 to "high" output
  delay(1000);              // 1 second delay
}
