#include <Arduino.h>
// Settings:
#define freq 50 // Circuit frequency in Hz (period of 16.67 milliseconds)

//Set initial values of t_on and t_off
float t_on = 12.5; // on period of the pulse in milliseconds
float t_off = 12.5; // off period of the pulse in milliseconds

// This code runs one time on startup or reset.
void setup() {

  //Set the DIGITAL pin 3 as an output pin
  pinMode(3, OUTPUT); //Note that DIGITAL Pins 3, 5, 6, 9, 10, 11 are PWM signals
}
//This code loops forever after setup is completed.
void loop() {
  digitalWrite(3, HIGH); // LED turns on
  delay(t_on); //LED stays on
  digitalWrite(3, LOW); // LED turns off
  delay(t_off); //LED stays off
}
