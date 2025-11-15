#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include<LiquidCrystal_I2C.h>
#include<Wire.h>

#define LED_PIN 5
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define ADDR 0x27

// Total times for tasks
const TickType_t ledTaskExecutionTime = 500 / portTICK_PERIOD_MS;      // 500 ms
const TickType_t counterTaskExecutionTime = 2000 / portTICK_PERIOD_MS; // 2 seconds
const TickType_t alphabetTaskExecutionTime = 13000 / portTICK_PERIOD_MS; // 13 seconds
// Remaining Execution Times
volatile TickType_t remainingLedTime = ledTaskExecutionTime;
volatile TickType_t remainingCounterTime = counterTaskExecutionTime;
volatile TickType_t remainingAlphabetTime = alphabetTaskExecutionTime;
// Task Handles
TaskHandle_t ledTaskHandle = NULL;
TaskHandle_t counterTaskHandle = NULL;
TaskHandle_t alphaTaskHandle = NULL;


void ledTask(void *arg) {
  while (1) {
    if (remainingLedTime > 0) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      remainingLedTime -= pdMS_TO_TICKS(100);
    }
    vTaskSuspend(NULL);
  }
}

void counterTask(void *arg) {
  int count = 1;
  while (1) {
    if (remainingCounterTime > 0) {
      // lcd.setCursor(0, 0);
      if (count == 13) {
        lcd.setCursor(0, 1);
      }
      lcd.print(count);
      //lcd.print(' ');
  
      count = count % 20 + 1; 

      remainingCounterTime -= pdMS_TO_TICKS(100);
    }
    vTaskSuspend(NULL);
  }
}

void alphabetTask(void *arg) {
  char letter = 'A';
  while (1) {
    if (remainingAlphabetTime > 0) {
      Serial.print(letter);
      Serial.print(' ');

      letter++;
      if (letter > 'Z') letter = 'A';

      remainingAlphabetTime -= pdMS_TO_TICKS(500);
    }
    vTaskSuspend(NULL);
  }
}

void scheduleTasks(void *arg) {
  while (1) {
    // Grab current remaining times
    TickType_t currLed = remainingLedTime;
    TickType_t currCounter = remainingCounterTime;
    TickType_t currAlpha = remainingAlphabetTime;

    if (currLed <= 0 && currCounter <= 0 && currAlpha <= 0) { // reset if all done !!
      lcd.clear();
      lcd.setCursor(0, 0);
      Serial.println();

      remainingLedTime = ledTaskExecutionTime;
      remainingCounterTime = counterTaskExecutionTime;
      remainingAlphabetTime = alphabetTaskExecutionTime;
    }

    // Determine which task to run
    if (currLed > 0 && (currLed <= currCounter || currCounter <= 0) && (currLed <= currAlpha || currAlpha <= 0)) {
      vTaskResume(ledTaskHandle);
      vTaskDelay(pdMS_TO_TICKS(100));
    } else if (currCounter > 0 && (currCounter <= currLed || currLed <= 0) && (currCounter <= currAlpha || currAlpha <= 0)) {
      vTaskResume(counterTaskHandle);
      vTaskDelay(pdMS_TO_TICKS(100));
    } else if (currAlpha > 0 && (currAlpha <= currCounter || currCounter <= 0) && (currAlpha <= currLed || currLed <= 0)) {
      vTaskResume(alphaTaskHandle);
      vTaskDelay(pdMS_TO_TICKS(500));
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void setup() {
  // Initializations
  Wire.begin();
  lcd.init();
  lcd.backlight();
  delay(2);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);

  xTaskCreate(ledTask, "LED", 4096, NULL, 1, &ledTaskHandle);
  xTaskCreate(counterTask, "Counter", 4096, NULL, 1, &counterTaskHandle);
  xTaskCreate(alphabetTask, "Alphabet", 4096, NULL, 1, &alphaTaskHandle);

  // Suspend all tasks at beginning
  vTaskSuspend(ledTaskHandle);
  vTaskSuspend(counterTaskHandle);
  vTaskSuspend(alphaTaskHandle);

  // Create scheduler to manage the other tasks
  xTaskCreate(scheduleTasks, "Scheduler", 4096, NULL, 2, NULL);

}

void loop() {
  // put your main code here, to run repeatedly:

}
