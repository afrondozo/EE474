// Filename: Lab2Part1.ino
// Authors: Aidan Frondozo, Kylie Neal
// Date: 10/22/2025
// Description: This file creates a blinking LED using direct GPIO register access

#include "driver/gpio.h"            // For GPIO driver-level macros
#include "soc/io_mux_reg.h"         // For configuring the MUX (multiplexer)
#include "soc/gpio_reg.h"           // For direct GPIO register access
#include "soc/gpio_periph.h"        // For GPIO pin definitions

// --- Constants --- //
#define GPIO_PIN 5    // GPIO pin connected to the LED

void setup() {
  // configure pin 5 as GPIO
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[GPIO_PIN], PIN_FUNC_GPIO);
  // set pin 5 as output pin
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << GPIO_PIN); 
}

void loop() {
  // set pin 5 high
  *((volatile uint32_t *)GPIO_OUT_REG) |= (1 << GPIO_PIN);

  // Wait for 1 second
  delay(1000);

  // set pin 5 low
  *((volatile uint32_t *)GPIO_OUT_REG) &= ~(1 << GPIO_PIN);

  // Wait again
  delay(1000);
}
