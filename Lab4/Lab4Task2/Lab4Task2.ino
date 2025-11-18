  // Filename: Lab4Task2.ino
  // Authors: Aidan Frondozo, Kylie Neal

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
  void ledAlarm(void *arg) {
    while (1) {
      // trigger an LED if currentLightLevel is below thresh
      if (xSemaphoreTake(dataReady, portMAX_DELAY) == pdTRUE) {
        if (currSMA > 3800 || currSMA < 300) {
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
