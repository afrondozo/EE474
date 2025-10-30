// Filename: Lab2Part1B.in
// Authors: Aidan Frondozo, Kylie Neal
// Date: 10/22/2025
// Description: This program compares the runtimes between using the Arduino functions
// and direct register access.

#include "driver/gpio.h"            // For GPIO driver-level macros
#include "soc/io_mux_reg.h"         // For configuring the MUX (multiplexer)
#include "soc/gpio_reg.h"           // For direct GPIO register access
#include "soc/gpio_periph.h"        // For GPIO pin definitions

#define GPIO_PIN 5

unsigned long regTime;
unsigned long methodTime;

void setup() {
  // configure pin 5 as GPIO output
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[GPIO_PIN], PIN_FUNC_GPIO);
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << GPIO_PIN);
  Serial.begin(9600);

}
/* Part 1 */
/*
void loop() {
  // ================> TODO:
  // For 1000 repetitions:
  //		Measure time to:
  //        		- Turn pin's output to HIGH
  //        		- Turn pin's output to LOW
  // Print out total time to the serial monitor
  // 1 second delay
  for (int i = 0; i < 1000; i++) {
    digitalWrite(19, HIGH);
    digitalWrite(19, LOW);
  }

  Serial.print("Total time: ");
  Serial.println(micros());
  delay(1000);
}
*/

void loop() {
  delay(5000);
  regTime = micros();
  for (int i = 0; i < 1000; i++) {
    *((volatile uint32_t *)GPIO_OUT_REG) |= (1 << GPIO_PIN); // high
    *((volatile uint32_t *)GPIO_OUT_REG) &= ~(1 << GPIO_PIN); // low
  }
  regTime = micros() - regTime;
  Serial.print("Total time using direct register: ");
  Serial.println(regTime);

  methodTime = micros();
  for (int i = 0; i < 1000; i++) {
    digitalWrite(GPIO_PIN, HIGH);
    digitalWrite(GPIO_PIN, LOW);
  }

  methodTime = micros() - methodTime;
  Serial.print("Total time using Arduino methods: ");
  Serial.println(methodTime);
}
