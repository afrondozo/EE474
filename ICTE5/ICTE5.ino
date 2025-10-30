// Filename: ICTE5.ino
// Authors: Kylie Neal, Aidan Frondozo

// Part 1:
/*
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
#define LED2_TOGGLE_INTERVAL 2000000

// Track last toggle time
static uint32_t last_toggle_time_LED1 = 0;
static uint32_t last_toggle_time_LED2 = 0;

typedef enum {
  READY,
  RUNNING,
  BLOCKED
} Status;

typedef struct {
  void (*TaskFunc)(void);
  Status status;
  char task_name[20];
} TCBStruct;

void TaskA(void) {
  // Read current timer value
  uint32_t current_time = *((volatile uint32_t *)TIMG_T0LO_REG(0));

  // Check if toggle interval has passed
  if ((current_time - last_toggle_time_LED1) >= LED1_TOGGLE_INTERVAL) {
    // Read current GPIO output state
    uint32_t gpio_out = *((volatile uint32_t *)GPIO_OUT_REG);

    //  Toggle GPIO_PIN using XOR
    *((volatile uint32_t *)GPIO_OUT_REG) = gpio_out ^ (1 << OUTPUT1);

    // Update last_toggle_time
    last_toggle_time_LED1 = current_time;
  }
}

void TaskB(void) {
  uint32_t current_time = *((volatile uint32_t *)TIMG_T0LO_REG(0));

  // Check if toggle interval has passed
  if ((current_time - last_toggle_time_LED2) >= LED2_TOGGLE_INTERVAL) {
    // Read current GPIO output state
    uint32_t gpio_out = *((volatile uint32_t *)GPIO_OUT_REG);

    //  Toggle GPIO_PIN using XOR
    *((volatile uint32_t *)GPIO_OUT_REG) = gpio_out ^ (1 << OUTPUT2);

    // Update last_toggle_time
    last_toggle_time_LED2 = current_time;
  }
}

TCBStruct task_list[2];

void init_tasks() {
  task_list[0] = (TCBStruct) { .TaskFunc = TaskA, .status = READY, .task_name = "TaskA"};
  task_list[1] = (TCBStruct) { .TaskFunc = TaskB, .status = READY, .task_name = "TaskB"};
}

void scheduler() {
  for (int i = 0; i < 2; i++) {
    if (task_list[i].status == READY) {
      task_list[i].status = RUNNING;
      Serial.println(task_list[i].task_name);
      task_list[i].TaskFunc();
      task_list[i].status = READY;
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

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  scheduler();
  *((volatile uint32_t *)TIMG_T0UPDATE_REG(0)) = 1;
}
*/

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
#define LED2_TOGGLE_INTERVAL 2000000

// Track last toggle time
static uint32_t last_toggle_time_LED1 = 0;
static uint32_t last_toggle_time_LED2 = 0;

typedef enum {
  READY,
  RUNNING,
  WAITING
} Status;

typedef struct {
  void (*TaskFunc)(void);
  Status status;
  char task_name[20];
  int delay;
  int last_time;
} TCBStruct;

void TaskA(void) {
    // Read current GPIO output state
    uint32_t gpio_out = *((volatile uint32_t *)GPIO_OUT_REG);
    //  Toggle GPIO_PIN using XOR
    *((volatile uint32_t *)GPIO_OUT_REG) = gpio_out ^ (1 << OUTPUT1);
}

void TaskB(void) {
    // Read current GPIO output state
    uint32_t gpio_out = *((volatile uint32_t *)GPIO_OUT_REG);
    //  Toggle GPIO_PIN using XOR
    *((volatile uint32_t *)GPIO_OUT_REG) = gpio_out ^ (1 << OUTPUT2);
}

TCBStruct task_list[2];

void init_tasks() {
  task_list[0] = (TCBStruct) { .TaskFunc = TaskA, .status = READY, .task_name = "TaskA", .delay = 10000000, .last_time = 0};
  task_list[1] = (TCBStruct) { .TaskFunc = TaskB, .status = READY, .task_name = "TaskB", .delay = 100000, .last_time = 0};
}

void scheduler() {
  // read current time
  uint32_t current_time = *((volatile uint32_t *)TIMG_T0LO_REG(0));

  for (int i = 0; i < 2; i++) {
    if ((current_time - task_list[i].last_time) >= task_list[i].delay) {
      task_list[i].status = RUNNING;
      task_list[i].TaskFunc();
      task_list[i].last_time = current_time;
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