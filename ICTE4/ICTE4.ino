// Filename: ICTE4.ino
// Authors: Aidan Frondozo, Kylie Neal
// Date: 10/15/2025
// Description: this file serves as the ICTE 4 round robin scheduler.

#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_periph.h"
#include "soc/timer_group_reg.h"

// define GPIO pins
#define GPIO_PIN1 5
#define GPIO_PIN2 6
#define GPIO_PIN3 7

// Define TIMER_INCREMENT_MODE and TIMER_ENABLE macros
#define TIMER_INCREMENT_MODE (1 << 30)
#define TIMER_ENABLE (1 << 31)

// set LED toggle times and potentiometer reading time
#define LED1_TOGGLE_INTERVAL 1000000
#define LED2_TOGGLE_INTERVAL 2000000
#define POT_TOGGLE_INTERVAL 3000000


// Track last toggle time
static uint32_t last_toggle_time_LED1 = 0;
static uint32_t last_toggle_time_LED2 = 0;
static uint32_t last_toggle_time_POT = 0;

void setup() {
  Serial.begin(9600);

  // set GPIO pins
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[GPIO_PIN1], PIN_FUNC_GPIO);
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[GPIO_PIN2], PIN_FUNC_GPIO);

  // enable GPIO as output
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << GPIO_PIN1);
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << GPIO_PIN2);

  // Configure timer
  uint32_t timer_config = 0;

  // apply a clock divider
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
  // put your main code here, to run repeatedly:
  TaskA();
  TaskB();
  TaskC();
  *((volatile uint32_t *)TIMG_T0UPDATE_REG(0)) = 1;
}

void TaskA() {
  // Read current timer value
  uint32_t current_time = *((volatile uint32_t *)TIMG_T0LO_REG(0));

  // Check if toggle interval has passed
  if ((current_time - last_toggle_time_LED1) >= LED1_TOGGLE_INTERVAL) {
    // Read current GPIO output state
    uint32_t gpio_out = *((volatile uint32_t *)GPIO_OUT_REG);

    //  Toggle GPIO_PIN using XOR
    *((volatile uint32_t *)GPIO_OUT_REG) = gpio_out ^ (1 << GPIO_PIN1);

    // Update last_toggle_time
    last_toggle_time_LED1 = current_time;
  }
}

void TaskB() {
  uint32_t current_time = *((volatile uint32_t *)TIMG_T0LO_REG(0));

  // Check if toggle interval has passed
  if ((current_time - last_toggle_time_LED2) >= LED2_TOGGLE_INTERVAL) {
    // Read current GPIO output state
    uint32_t gpio_out = *((volatile uint32_t *)GPIO_OUT_REG);

    //  Toggle GPIO_PIN using XOR
    *((volatile uint32_t *)GPIO_OUT_REG) = gpio_out ^ (1 << GPIO_PIN2);

    // Update last_toggle_time
    last_toggle_time_LED2 = current_time;
  }
}

void TaskC() {
  // Read current timer value
  uint32_t current_time = *((volatile uint32_t *)TIMG_T0LO_REG(0));

  // Check if toggle interval has passed
  if ((current_time - last_toggle_time_POT) >= POT_TOGGLE_INTERVAL) {
    // Read current GPIO input
    int reading = analogRead(GPIO_PIN3);

    Serial.print("Pot reading = ");
    Serial.println(reading);

    // Update last_toggle_time
    last_toggle_time_POT = current_time;
  }
}
