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
SemaphoreHandle_t xBinarySemaphore;


// === CIRCULAR BUFFER ===
int buffer[5] = {0};
int bufferIndex = 0;

// === LIGHT DETECTOR ===
int currLightLevel = 0;
int currSMA = 0;

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
  }
}

void lcdDisplay(void *arg) {}

void ledAlarm(void *arg) {}

void primeNumberCalculator(void *arg) {}

void setup() {
  int initialLightReading = analogRead(PHOTO_RESISTOR);
  for (int i = 0; i < WINDOW_SIZE; i++) {
        buffer[i] = initial;
  }

  xBinarySemaphore = xSemaphoreCreateBinary();
  pinMode();

}

void loop() {}
