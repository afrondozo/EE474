// Filename: Lab4Task2.ino
// Authors: Aidan Frondozo, Kylie Neal
// Description: Implements a dual-core light level anomaly detector system.

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "LiquidCrystal_I2C.h"
#include "Wire.h"

#define PHOTO_RESISTOR 2
#define LED_PIN 1

// === LCD ===
LiquidCrystal_I2C lcd(0x27, 16, 2);

// === SEMAPHORE ===
SemaphoreHandle_t dataReady;

// === CIRCULAR BUFFER ===
int buffer[5] = {0};
int bufferIndex = 0;

// === LIGHT DETECTOR ===
int currLightLevel = 0;
int currSMA = 0;
int lcdLightLevel = 0;
int lcdSMA = 0;

/**
* @brief FreeRTOS task that reads the photoresistor value and computes
*        a 5-sample Simple Moving Average (SMA).
*
* Reads the current light level, updates the circular buffer, computes
* the SMA, waits 500 ms, and then signals other tasks via a semaphore
* that new data is available.
*
* @param arg Unused task parameter.
*/
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
    currSMA = sum / 5;
    vTaskDelay(pdMS_TO_TICKS(500));
    xSemaphoreGive(dataReady);
  }
}

// === LIGHT LEVEL DISPLAY ===
/**
* @brief Updates the LCD display with the current light level and SMA.
*
* Clears the LCD and writes two lines: the raw light reading and the
* computed simple moving average.
*
* @param lightLevel Current measured light intensity.
* @param SMA        Current 5-sample moving average value.
*/
void displayLightLevel(int lightLevel, int SMA) {
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("Light:");
  lcd.setCursor(7, 0);
  lcd.print(lightLevel);

  lcd.setCursor(0, 1);
  lcd.print("SMA:");
  lcd.setCursor(5, 1);
  lcd.print(SMA);
}

/**
 * @brief FreeRTOS task that prints updated light and SMA values to the LCD.
 *
 * Waits on the semaphore indicating new sensor data. If the values have
 * changed since the last display update, it refreshes the LCD using
 * displayLightLevel().
 *
 * @param arg Unused task parameter.
 */
void lcdDisplay(void *arg) {
  while (1) {
    if (xSemaphoreTake(dataReady, portMAX_DELAY) == pdTRUE && (lcdLightLevel != currLightLevel || lcdSMA != currSMA)) {
      // display current light level and sma
      lcdLightLevel = currLightLevel;
      lcdSMA = currSMA;
      
      displayLightLevel(lcdLightLevel, lcdSMA);
    }
  }
}

// === LIGHT LEVEL ALARM ===
/**
 * @brief FreeRTOS task that triggers an LED alarm when light levels
 *        are outside the acceptable range.
 *
 * When new light data is available, checks whether the reading is too
 * high or too low. If so, blinks the LED in a repeating alert pattern.
 *
 * @param arg Unused task parameter.
 */
void ledAlarm(void *arg) {
  while (1) {
    // trigger an LED if currentLightLevel is below threshold
    if (xSemaphoreTake(dataReady, portMAX_DELAY) == pdTRUE) {
      if (currLightLevel > 3800 || currLightLevel < 300) {
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(2000));
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(2000));
        digitalWrite(LED_PIN, HIGH);
        vTaskDelay(pdMS_TO_TICKS(500));
        digitalWrite(LED_PIN, LOW);
        vTaskDelay(pdMS_TO_TICKS(2000));
      }
    }
  }
}

// === PRIME NUMBERS ===
/**
 * @brief Determines whether a number is prime.
 *
 * Efficient primality check using early exits and testing only odd
 * divisors up to sqrt(n).
 *
 * @param n The integer to test.
 * @return true if the number is prime.
 * @return false otherwise.
 */
bool isPrime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0) return false;

    // Check odd divisors up to sqrt(n)
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }

    return true;
}

/**
 * @brief FreeRTOS task that prints all prime numbers from 2 to 50,000.
 *
 * Iterates through numbers, checks each for primality, and prints
 * primes to the serial monitor. Includes a delay between checks to
 * reduce CPU load.
 *
 * @param arg Unused task parameter.
 */
void primeNumberCalculator(void *arg) {
  while (1) {
    for (int i = 2; i <= 50000; i++) {
      if (isPrime(i)) {
        Serial.print(i);
        Serial.println(" is a prime number!");
      }
      vTaskDelay(pdMS_TO_TICKS(200));
    }
  }
}

/**
 * @brief Arduino setup function. Initializes hardware, tasks, semaphore,
 *        and LCD display.
 *
 * Configures GPIO, LCD, serial output, buffer initialization, and
 * creates four tasks pinned to specific CPU cores.
 */
void setup() {
  // initialize pins
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(PHOTO_RESISTOR, INPUT);

  // initialize LCD
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.display();
  delay(2);

  // initialize serial monitor
  Serial.begin(115200);

  // initialize the buffer with light level values
  int initialLightReading = analogRead(PHOTO_RESISTOR);
  for (int i = 0; i < 5; i++) {
        buffer[i] = initialLightReading;
  }

  // initialize semaphore
  dataReady = xSemaphoreCreateBinary();

  // pin tasks to core
  xTaskCreatePinnedToCore(lightDetector, "Light Detector Task", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(lcdDisplay, "LCD", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(ledAlarm, "LED Alarm", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(primeNumberCalculator, "Prime Numbers", 4096, NULL, 1, NULL, 1);

  delay(2000);
}

void loop() {}
