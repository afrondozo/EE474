// Filename: ICE2.ino
// Author: Aidan Frondozo, Kylie Neal
// Date: 10/08/2025
// Description: Part 1 uses the GPIO functions to blink an LED. Part 2 uses timers to blink an LED.

/*========== ICE2 PART 1 ==========*/
/*
#include "driver/gpio.h"            // For GPIO driver-level macros
#include "soc/io_mux_reg.h"         // For configuring the MUX (multiplexer)
#include "soc/gpio_reg.h"           // For direct GPIO register access
#include "soc/gpio_periph.h"        // For GPIO pin definitions

// --- Constants --- //
#define GPIO_PIN 5    // GPIO pin connected to the LED

void setup() {
  // --- TODO: Set the pin function to GPIO using the appropriate macro --- //
  // Hint: Use a macro to select the GPIO function for GPIO_PIN
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[GPIO_PIN], PIN_FUNC_GPIO);

  // --- TODO: Enable the GPIO pin as an output --- //
  // Hint: Use GPIO_ENABLE_REG and a bitwise OR operation
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << GPIO_PIN);
   
}

void loop() {
  // --- TODO: Turn the LED ON --- //
  // Hint: Set the output register bit corresponding to GPIO_PIN to 1
  *((volatile uint32_t *)GPIO_OUT_REG) |= (1 << GPIO_PIN);

  // Wait for 1 second
  delay(1000);

  // --- TODO: Turn the LED OFF --- //
  // Hint: Clear the output register bit corresponding to GPIO_PIN
  *((volatile uint32_t *)GPIO_OUT_REG) &= ~(1 << GPIO_PIN);

  // Wait again
  delay(1000);
}
*/



/*========== ICE2 PART 2 ==========*/
#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_periph.h"
#include "soc/timer_group_reg.h"

// Define GPIO pin number
#define GPIO_PIN 5

// Define toggle interval in timer ticks (e.g., 1 second)
#define LED_TOGGLE_INTERVAL 1000000

// Define TIMER_INCREMENT_MODE and TIMER_ENABLE macros
#define TIMER_INCREMENT_MODE (1 << 30)
#define TIMER_ENABLE (1 << 31)

void setup() {
  // Set GPIO_PIN function to GPIO using MUX macro
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[GPIO_PIN], PIN_FUNC_GPIO);

  // Enable GPIO_PIN as output
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << GPIO_PIN);

  // Configure timer
  uint32_t timer_config = 0;

  // Optionally apply a clock divider
  timer_config |= (80 << 13);

  // Set increment mode and enable timer
  timer_config |= TIMER_INCREMENT_MODE;
  timer_config |= TIMER_ENABLE;

  // Write config to timer register
  *((volatile uint32_t *)TIMG_T0CONFIG_REG(0)) = timer_config;

  // Trigger a timer update to load settings
  *((volatile uint32_t *)TIMG_T0UPDATE_REG(0)) = 1;
}

void loop() {
  // Track last toggle time
  static uint32_t last_toggle_time = 0;

  // Read current timer value
  uint32_t current_time = 0;
  current_time = *((volatile uint32_t *)TIMG_T0LO_REG(0));

  // Check if toggle interval has passed
  if ((current_time - last_toggle_time) >= LED_TOGGLE_INTERVAL) {
    // Read current GPIO output state
    uint32_t gpio_out = 0;
    gpio_out = *((volatile uint32_t *)GPIO_OUT_REG);

    //  Toggle GPIO_PIN using XOR
    *((volatile uint32_t *)GPIO_OUT_REG) = gpio_out ^ (1 << GPIO_PIN);

    // Update last_toggle_time
    last_toggle_time = current_time;
  }

  // Refresh timer counter value
  *((volatile uint32_t *)TIMG_T0UPDATE_REG(0)) = 1;
}