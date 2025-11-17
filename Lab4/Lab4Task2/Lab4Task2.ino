// Filename: Lab4Task2.ino
// Authors: Aidan Frondozo, Kylie Neal

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "LiquidCrystal_I2C.h"
#include "Wire.h"

#define PHOTO_RESISTOR 2
#define SCL 4
#define SDA 5
#define LED_PIN 1

// === SEMAPHORE ===
SemaphoreHandle_t dataReady;

// === CIRCULAR BUFFER ===
int buffer[5] = {0};
int bufferIndex = 0;

// === LIGHT DETECTOR ===
int currLightLevel = 0;
int currSMA = 0;
int lcdLightLevel = 0;

void lightDetector(void *arg) {
  while (1) {
    int reading = analogRead(PHOTO_RESISTOR);
    currLightLevel = reading;

    buffer[bufferIndex] = reading;
    bufferIndex = (bufferIndex + 1) % 5;
    
    int sum = 0;
    for (int i = 0; i < 5; i++) {
      sum += buffer[i];
    }
    currSMA = sum / 5

    xSemaphoreGive(dataReady);
    xSemaphoreGive(dataReady);
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// === LIGHT LEVEL DISPLAY ===
void lcdDisplay(void *arg) {
  while (1) {
    if (xSemaphoreTake(dataReady, portMAX_DELAY) == pdTRUE & lcdLightLevel != currLightLevel) {
      // display current light level and sma
      lcdLightLevel = currLightLevel;
      Serial.print("Light level: "); // test this works first
      Serial.println(lcdLightLevel);
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

// === LIGHT LEVEL ALARM ===
void ledAlarm(void *arg) {
  while (1) {
    // trigger an LED if currentLightLevel is below thresh
  }
}

// === PRIME NUMBERS ===
void primeNumberCalculator(void *arg) {
  while (1) {
    for (int i = 2; i <= 50000; n++) {
      // calculate prime numbers
    }
  }
}

void setup() {
  // initialize LCD
  bool i2c_init = Wire.setPins(SDA, SCL);
  Wire.begin();
  lcd.init();
  lcd.display();

  // initialize the buffer with light level values
  int initialLightReading = analogRead(PHOTO_RESISTOR);
  for (int i = 0; i < WINDOW_SIZE; i++) {
        buffer[i] = initial;
  }

  // initialize semaphore
  dataReady = xSemaphoreCreateBinary();
  pinMode();

}

void loop() {}
