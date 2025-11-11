#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include<LiquidCrystal_I2C.h>
#include<Wire.h>

#define LED_PIN 5
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Total times for tasks
const TickType_t ledTaskExecutionTime = 500 / portTICK_PERIOD_MS;      // 500 ms
const TickType_t counterTaskExecutionTime = 2000 / portTICK_PERIOD_MS; // 2 seconds
const TickType_t alphabetTaskExecutionTime = 13000 / portTICK_PERIOD_MS; // 13 seconds
// Remaining Execution Times
volatile TickType_t remainingLedTime = ledTaskExecutionTime;
volatile TickType_t remainingCounterTime = counterTaskExecutionTime;
volatile TickType_t remainingAlphabetTime = alphabetTaskExecutionTime;


void ledTask(void *arg) {
  remainingLedTime = ledTaskExecutionTime;
  while (remainingLedTime > 0) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    vTaskDelay(pdMS_TO_TICKS(100));
    remainingLedTime -= 100;
  }
}

void counterTask(void *arg) {
 // TODO: Print out an incrementing counter to your LCD, and 
 //       update remaining time for this task
  remainingCounterTime = counterTaskExecutionTime;
  while (1) {
    sendCommand(0x01); // clear lcd
    sendCommand(0x80); // set cursor to beg

    for (int i = 1; i < 10; i++) {
      if (remainingCounterTime > 0) {
        lcd.print('0' + i); 
        vTaskDelay(pdMS_TO_TICKS(100));
        remainingCounterTime -= 100;
      }
    }

    for (int i = 0; i < 10; i++) {
      if (remainingCounterTime > 0) {
        lcd.print('1');
        lcd.print('0' + i); 
        vTaskDelay(pdMS_TO_TICKS(100));
        remainingCounterTime -= 100;
      }
    }

    if (remainingCounterTime > 0) {
        lcd.print('2');
        lcd.print('0'); 
        vTaskDelay(pdMS_TO_TICKS(100));
        remainingCounterTime -= 100;
    } 
  }
}

void alphabetTask(void *arg) {
 // TODO: Print out the alphabet to Serial, and update remaining
 //       time for this task
  remainingAlphabetTime = alphabetTaskExecutionTime;
  while (1) {
    for (char c = 'A'; c <= 'Z'; c++) {
      if (remainingAlphabetTime > 0)
        Serial.print(c);
        Serial.print(' ');
        vTaskDelay(pdMS_TO_TICKS(500));
        remainingCounterTime -= 500;
      }
    }
    Serial.println();
  }
}

void scheduleTasks(void *arg) {
   // TODO: Implement SRTF scheduling logic. This function should select the task with 
   //       the shortest remaining time and run it. Once a task completes it should 
   //       reset its remaining time.
}

void setup() {
  lcd.init();
  lcd.display();
  delay(2);
  pinMode(LED_PIN, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

}
