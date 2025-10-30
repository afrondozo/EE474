// Filename: Lab1Part3.ino
// Author: Aidan Frondozo
// Date: 10/06/2025
// Description: This file controls a blinking LED with a toggle button using an ESP32

/* =========== GLOBAL VARIABLES ===========*/
int state = 1;
int inputPin = 40;
int reading;
int previous;

unsigned long lastToggleTime = 0;           // the last time the output pin was toggled
unsigned long debounce = 200UL;   // the debounce time, increase if the output flickers

void setup() {
  // put your setup code here, to run once:
  pinMode(19, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(inputPin, INPUT_PULLUP);

}

void loop() {
  reading = digitalRead(inputPin); // gets button input

  // if button is pressed and was previously off change state.
  if (reading == HIGH && previous == LOW && millis() - lastToggleTime > debounce) {
    if (state) {
      state = 0;
    } else {
      state = 1;
    }

    lastToggleTime = millis();
  }

  blink(state); // blink LED according to the current state

  previous = reading;
  
}

// Name: blink
// Description: blinks and LED according to the current state.
void blink(int state) {
  if (state) {
    digitalWrite(19, HIGH);
    delay(500);
    digitalWrite(19, LOW);
    delay(500);
  } else {
    digitalWrite(19, LOW);
    delay(1000);
  }
}