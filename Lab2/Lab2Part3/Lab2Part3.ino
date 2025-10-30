// Filename: Lab2Part3.ino
// Authors: Aidan Frondozo, Kylie Neal
// Date: 10/22/2025
// Description: this program increases an LED's brightness when the light level gets low and 
// decreases it when the light level increases.

#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_periph.h"

#define INPUT_PIN 4
#define OUTPUT_PIN 41

void setup() {
  // set GPIO pin
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[OUTPUT_PIN], PIN_FUNC_GPIO);

  // enable GPIO as output
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << OUTPUT_PIN);

  // configure pin for PWM
  ledcAttach(OUTPUT_PIN, 300, 12);

  Serial.begin(9600);

}

void loop() {
  // get light level
  int reading = analogRead(INPUT_PIN);
  Serial.println(reading);
  ledcWrite(OUTPUT_PIN, 4000 - reading);
}
