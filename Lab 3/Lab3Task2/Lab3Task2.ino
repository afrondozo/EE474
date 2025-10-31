// Filename: Lab3Task2.ino
// Authors: Kylie Neal, Aidan Frondozo

// Part 2:
#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_periph.h"
#include "soc/timer_group_reg.h"

#define OUTPUT1 5
#define OUTPUT2 6

// Define TIMER_INCREMENT_MODE and TIMER_ENABLE macros
#define TIMER_INCREMENT_MODE (1 << 30)
#define TIMER_ENABLE (1 << 31)

// set LED toggle times and potentiometer reading time
#define LED1_TOGGLE_INTERVAL 1000000

// Track last toggle time
static uint32_t last_toggle_time_LED1 = 0;

typedef enum {
  READY,
  RUNNING,
  WAITING
} Status;

typedef struct {
  void (*TaskFunc)(void);
  Status status;
  char task_name[20];
  int priority;
} TCBStruct;

void TaskA(void) {
    // Read current GPIO output state
    // Toggle GPIO_PIN using XOR
    for (int i = 0; i < 16; i ++) {
      uint32_t gpio_out = *((volatile uint32_t *)GPIO_OUT_REG);
      *((volatile uint32_t *)GPIO_OUT_REG) = gpio_out ^ (1 << OUTPUT1);
      delay(500'000);
    }
}

void Task_C(void) {
    int[] notes = {NOTE_Bb, NOTE_B, NOTE_C, NOTE_Cs, NOTE_D, NOTE_Eb, NOTE_E, NOTE_F, NOTE_Fs, NOTE_G} 
    for (int i = 0; i < 10; i++) {
      ledcWriteNote(OUTPUT2, note[i], 6); 
      delay(400000);
    }
}

void TaskD(void) {
    for (char c = 'A'; c <= 'Z'; c++) {
        Serial.println(c);
    }
}

TCBStruct task_list[4];

void init_tasks() {
  task_list[0] = (TCBStruct) { .TaskFunc = TaskA, .status = READY, .task_name = "LED Blinker", .priority = 1};
  task_list[1] = (TCBStruct) { .TaskFunc = TaskB, .status = READY, .task_name = "LCD Counter", .priority = 2};
  task_list[2] = (TCBStruct) { .TaskFunc = TaskC, .status = READY, .task_name = "Buzzer Notes", .priority = 3};
  task_list[3] = (TCBStruct) { .TaskFunc = TaskD, .status = READY, .task_name = "Serial Alpha", .priority = 4};
}

void scheduler() {
  // read current time
  uint32_t current_time = *((volatile uint32_t *)TIMG_T0LO_REG(0));

  for (int i = 0; i < 4; i++) {
    if ((current_time - task_list[i].last_time) >= task_list[i].delay) {
      task_list[i].status = RUNNING;
      task_list[i].TaskFunc();
      task_list[i].status = WAITING;
    }
  }
}

void setup() {
  // set GPIO pins
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[OUTPUT1], PIN_FUNC_GPIO);
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[OUTPUT2], PIN_FUNC_GPIO);

  // enable GPIO as output
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << OUTPUT1);
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << OUTPUT2);

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

  // init tasks
  init_tasks();
}

void loop() {
  scheduler();
  *((volatile uint32_t *)TIMG_T0UPDATE_REG(0)) = 1;
}