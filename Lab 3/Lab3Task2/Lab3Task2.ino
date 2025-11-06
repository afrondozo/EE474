// Filename: Lab3Task2.ino
// Authors: Kylie Neal, Aidan Frondozo
// Date: 11/05/2025
// Description: This file implements a task scheduler with priorities.

// ============== INCLUDES ==============
#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_periph.h"
#include "soc/timer_group_reg.h"
#include<Wire.h>
#include<LiquidCrystal_I2C.h>

// ============== MACROS ==============
#define OUTPUT1 5
#define OUTPUT2 6
#define ADDR 0x27

LiquidCrystal_I2C lcd(0x27, 16, 2);
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

// ============== FUNCTION PROTOTYPES ==============
// Name: TaskA
// Description: Toggles an LED 8 times, once per second
void TaskA(void) {
    // Read current GPIO output state, toggle GPIO_PIN using XOR
    for (int i = 0; i < 16; i ++) {
      uint32_t gpio_out = *((volatile uint32_t *)GPIO_OUT_REG);
      *((volatile uint32_t *)GPIO_OUT_REG) = gpio_out ^ (1 << OUTPUT1);
      delay(500);
    }
}

// Name: TaskB
// Description: Counts from 1 to 10 on the LCD Screen
void TaskB(void) {
  sendCommand(0x01); // clear lcd
  sendCommand(0x80); // set cursor to beg
  delay(500);
  for (int i = 1; i < 10; i++) {
    writeLCD('0' + i); 
    delay(1000);
  }
  writeLCD('1');
  writeLCD('0');
}

// Name: TaskC
// Description: Plays 10 notes (ascending Cs and Gs) on the buzzer
void TaskC(void) {
    note_t notes[] = {NOTE_C, NOTE_G}; 
    for (int i = 0; i < 10; i++) {
      ledcWriteNote(OUTPUT2, notes[i % 2], (i / 2) + 4); 
      delay(800);
    }

    ledcWrite(OUTPUT2, 0);
}

// Name: TaskD
// Description: Prints alphabet onto serial monitor
void TaskD(void) {
    for (char c = 'A'; c <= 'Z'; c++) {
        Serial.print(c);
        Serial.print(' ');
    }
    Serial.println();
}

// Name: init_tasks
// Description: Initializes the 4 tasks A-D with initial priorities
TCBStruct task_list[4];
void init_tasks() {
  task_list[0] = (TCBStruct) { .TaskFunc = TaskA, .status = READY, .task_name = "LED Blinker", .priority = 1};
  task_list[1] = (TCBStruct) { .TaskFunc = TaskB, .status = READY, .task_name = "LCD Counter", .priority = 2};
  task_list[2] = (TCBStruct) { .TaskFunc = TaskC, .status = READY, .task_name = "Buzzer Notes", .priority = 3};
  task_list[3] = (TCBStruct) { .TaskFunc = TaskD, .status = READY, .task_name = "Serial Alphabet", .priority = 4};
}

// Name: scheduler
// Description: Runs tasks based on priority. Resets priority to next priority.
void scheduler() {
  // Runs next set of tasks
  for (int i = 1; i < 5; i++) { // priority
    for (int j = 0; j < 4; j++) { // tasks
      if (task_list[j].status == READY && task_list[j].priority == i) {
        task_list[j].status = RUNNING;
        task_list[j].TaskFunc();
        task_list[j].status = WAITING;
        Serial.print(task_list[j].task_name);
        Serial.print(": ");
        Serial.println(task_list[j].priority);
      }
    }
  }

  // Resets priorities
  for (int i = 0; i < 4; i++) {
    task_list[i].priority = task_list[i].priority % 4 + 1;
    task_list[i].status = READY;
  }
}

// Name: writeLCD
// Description: Writes the given data to the LCD
void writeLCD(uint8_t data) {
  uint8_t high = data & 0xF0;
  uint8_t low = data << 4;
  Wire.beginTransmission(ADDR);
  Wire.write(high | 0x0D);  // bit 3 = backlight on
  Wire.write((high & ~0x04) | 0x09);  // bit 3 = backlight on
  Wire.write(low | 0x0D);  // bit 3 = backlight on
  Wire.write((low & ~0x04) | 0x09);  // bit 3 = backlight on
  Wire.endTransmission();
}

// Name: sendCommand
// Description: Sends the given command to the LCD
void sendCommand(uint8_t data) {
  int8_t high = data & 0xF0;
  uint8_t low = data << 4;
  Wire.beginTransmission(ADDR);
  Wire.write(high | 0x0C);  // bit 3 = backlight on
  Wire.write((high & ~0x04) | 0x08);  // bit 3 = backlight on
  Wire.write(low | 0x0C);  // bit 3 = backlight on
  Wire.write((low & ~0x04) | 0x08);  // bit 3 = backlight on
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  lcd.init();
  sendCommand(0x0C);
  Wire.begin();
  delay(2);

  // set GPIO pins
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[OUTPUT1], PIN_FUNC_GPIO);
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[OUTPUT2], PIN_FUNC_GPIO);

  // enable registers
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << OUTPUT1);
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << OUTPUT2);

  // Initialize buzzer and LED2 output
  ledcAttach(OUTPUT2, 1000, 8);

  // init tasks
  init_tasks();
}

void loop() {
  scheduler();
}
