// Filename: Lab2Part4
// Authors: Aidan Frondozo, Kylie Neal
// Date: 10/20/2025
// Description: this program turns on a buzzer sequence when the light levels reach a certain 
//              threshold.

#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_periph.h"
#include "soc/timer_group_reg.h"

// define input/output pins
#define INPUT_PIN 4
#define OUTPUT_PIN 41

// define TIMER_INCREMENT_MODE and TIMER_ENABLE macros
#define TIMER_INCREMENT_MODE (1 << 30)
#define TIMER_ENABLE (1 << 31)

// define buzzer time segments
#define BUZZER_INTERVAL 1500000

// define light threshold
#define THRESHOLD 2000


void setup() {
  Serial.begin(9600);

  // set GPIO PIN
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[OUTPUT_PIN], PIN_FUNC_GPIO);

  // enable GPIO as output
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << OUTPUT_PIN);

  // configure pin for PWM
  ledcAttach(OUTPUT_PIN, 1000, 8);

  // Configure timer
  uint32_t timer_config = 0;

  // apply a clock divider
  timer_config |= (80 << 13);

  // Set increment mode and enable timer
  timer_config |= TIMER_INCREMENT_MODE;
  timer_config |= TIMER_ENABLE;
  
  // set timer load register to 0
  *((volatile uint32_t *)TIMG_T0LOADLO_REG(0)) = 0;
  *((volatile uint32_t *)TIMG_T0LOADHI_REG(0)) = 0;

  // Write config to timer register
  *((volatile uint32_t *)TIMG_T0CONFIG_REG(0)) = timer_config;

  // Trigger a timer update to load settings
  *((volatile uint32_t *)TIMG_T0UPDATE_REG(0)) = 1;
}

void loop() {
  // light level input
  int light_level = analogRead(INPUT_PIN);
  if (light_level < THRESHOLD) {
    // reset timer to 0
    *((volatile uint32_t *)TIMG_T0LOAD_REG(0)) = 1;
    *((volatile uint32_t *)TIMG_T0UPDATE_REG(0)) = 1;

    // read current time
    uint32_t current_time = *((volatile uint32_t *)TIMG_T0LO_REG(0));

    while (current_time < BUZZER_INTERVAL) {
      if (current_time < 500000) {
        Serial.println("Playing low note");
        ledcWriteNote(OUTPUT_PIN, NOTE_C, 5); 
      } else if (current_time > 500000 && current_time < 1000000) {
        Serial.println("Playing mid note");
        ledcWriteNote(OUTPUT_PIN, NOTE_F, 5);
      } else {
        Serial.println("Playing high note");
        ledcWriteNote(OUTPUT_PIN, NOTE_B, 5);
      }
      // update timer
      *((volatile uint32_t *)TIMG_T0UPDATE_REG(0)) = 1;
      current_time = *((volatile uint32_t *)TIMG_T0LO_REG(0));
    }
  } else {
    ledcWrite(OUTPUT_PIN, 0);
  }
}





