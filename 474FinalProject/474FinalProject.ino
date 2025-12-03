// Filename: 474FinalProject.ino
// Authors: Kylie Neal, Aidan Frondozo
// Description: implements a smart rfid security system

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "LiquidCrystal_I2C.h"
#include "Wire.h"
#include "AM2320_asukiaaa.h"
#include <SPI.h>
#include <MFRC522.h>
#include <RTClib.h>

#define SS_PIN  5  
#define RST_PIN 11 
#define RED_LED 7
#define GREEN_LED 6
#define TOGGLE_INTERVAL_GREEN 2000000  // 2 seconds
#define TOGGLE_INTERVAL_RED 2000000  // 2 seconds

const byte correctUID[] = {0xAA, 0x22, 0x0A, 0x01};
const byte correctUIDLength = 4;

MFRC522 rfid(SS_PIN, RST_PIN);
RTC_DS3231 rtc;
DateTime now;

esp_timer_handle_t green_timer;
esp_timer_handle_t red_timer;

void IRAM_ATTR greenOnTimer(void* arg) {
  digitalWrite(GREEN_LED, 0);
}

void IRAM_ATTR redOnTimer(void* arg) {
  digitalWrite(RED_LED, 0);
}

char daysOfWeek[7][12] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};


TaskHandle_t clockTaskHandle = NULL;
TaskHandle_t rfidTaskHandle = NULL;

// === LCD ===
LiquidCrystal_I2C lcd(0x27, 16, 2);

// === SENSOR SEMAPHORE ===
SemaphoreHandle_t dataReady;

// === LCD SEMAPHORE ===
SemaphoreHandle_t lcdReady;

// === TEMPERATURE SENSOR ===
AM2320_asukiaaa tempSensor;
float temperature = 0;
float humidity = 0;

// === TOGGLE BUTTON ===
#define button 1
SemaphoreHandle_t buttonSemaphore;
volatile int buttonState = 0;  // shared state, toggled by button task

/**
* @brief Helper method for outputting a temperature display on LCD
*
* Displays current outdoor temperature reading on LCD
* @param temp current temperature
*/
void displayTemp (float temp) {
  // lcd.clear();
  // lcd.print("Temperature:");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Temp: " + String(temp) + "F");
}
/**
* @brief Helper method for outputting a humidity display on LCD
*
* Displays current outdoor humidity reading on LCD
* @param humid current humidity
*/
void displayHumidity (float humid) {
  // lcd.clear();
  // lcd.print("Humidity:");
  // lcd.setCursor(0, 1);
  // lcd.print(String(humid) + "%");
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  lcd.print("Humidity: " + String(humidity) + "%");
}

bool compareUID(byte *uid, byte uidLength) {
  if (uidLength != correctUIDLength) return false;
  for (int i = 0; i < uidLength; i++) {
    if (uid[i] != correctUID[i]) return false;
  }
  return true;
}

void printTime() {
  // DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.println(now.second(), DEC);
}

/**
 * @brief FreeRTOS task to read data from the AM2320 sensor.
 * 
 * This task periodically updates the temperature and humidity readings 
 * from the sensor. After each update, it signals other tasks that new 
 * data is available by giving the `dataReady` semaphore.
 * 
 * @param args Pointer to task parameters (not used).
 * 
 * @note Uses the global variables `temperature`, `humidity`, and the binary 
 *       semaphore `dataReady`.
 */
void TASK_SensorData (void *args) {
  while (1) {
    // Serial.println("Getting sensor data");
    tempSensor.update();
    temperature = tempSensor.temperatureF;
    humidity = tempSensor.humidity;
    vTaskDelay(pdMS_TO_TICKS(1000));
    xSemaphoreGive(dataReady);  
  }
}
/**
 * @brief FreeRTOS task to display temperature or humidity on the LCD.
 * 
 * This task waits for the `dataReady` semaphore to be given by the sensor task,
 * indicating that new sensor data is available. It then reads the latest
 * temperature and humidity values and updates the LCD display. The displayed
 * value depends on the global `buttonState` variable:
 * - `true`  : display temperature
 * - `false` : display humidity
 * 
 * @param args Pointer to task parameters (not used).
 * 
 * @note Uses the global variables `temperature`, `humidity`, and `buttonState`.
 * @note Uses the helper functions `displayTemp()` and `displayHumidity()` to update the LCD.
 */
void TASK_DisplayData (void *args) {
  while (1) {
    if (xSemaphoreTake(dataReady, portMAX_DELAY) == pdTRUE) {
      float currTemp = temperature;
      float currHumid = humidity;
      
      if (xSemaphoreTake(lcdReady, portMAX_DELAY) == pdTRUE) {
        if (buttonState) {
          displayTemp(currTemp);
          xSemaphoreGive(lcdReady);
        } else {
          displayHumidity(humidity);
          xSemaphoreGive(lcdReady);
        }
      }
    }
  }
}
/**
 * @brief FreeRTOS task to handle button presses.
 * 
 * This task waits for the `buttonSemaphore` to be given by the button ISR,
 * indicating that the user has pressed the button. When the semaphore is taken,
 * the task toggles the global `buttonState` variable, which controls whether
 * temperature or humidity is displayed on the LCD.
 * 
 * @param args Pointer to task parameters (not used).
 * 
 * @note Runs indefinitely in a while(1) loop.
 * @note Uses the global variables `buttonState` and `buttonSemaphore`.
 */
void TASK_ButtonHandler(void *args) {
  const unsigned long debounceDelay = 200; // milliseconds
  static unsigned long lastPressTime = 0;
  while (1) {
    if (xSemaphoreTake(buttonSemaphore, portMAX_DELAY) == pdTRUE) {
      unsigned long currentTime = millis();
      if (currentTime - lastPressTime > debounceDelay) {
          // toggle display mode
          buttonState ^= 1;
          lastPressTime = currentTime;
          //Serial.println(buttonState ? "Temperature" : "Humidity");
      }
    }
  }
}
/**
 * @brief Interrupt Service Routine (ISR) for the button press.
 * 
 * This ISR is attached to a GPIO pin connected to the button. When the button
 * is pressed, the ISR gives the `buttonSemaphore` to unblock the `TASK_ButtonHandler`
 * task. The ISR uses the FreeRTOS function `xSemaphoreGiveFromISR` to safely
 * interact with the semaphore from an interrupt context.
 * 
 * @note Uses `portYIELD_FROM_ISR` to request a context switch if the
 *       unblocked task has higher priority.
 * @note Should be as short as possible to avoid blocking other interrupts.
 */
void IRAM_ATTR handleButtonInterrupt() {
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  xSemaphoreGiveFromISR(buttonSemaphore, &xHigherPriorityTaskWoken);
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void clockTask(void* parameter) {
  while (1) {
    // Print time
    xSemaphoreTake(lcdReady, portMAX_DELAY);
    now = rtc.now();
    
    lcd.setCursor(0, 0);
    char timeStr[16];
    sprintf(timeStr, "%02d:%02d:%02d        ", now.hour(), now.minute(), now.second());
    lcd.print(timeStr);
    xSemaphoreGive(lcdReady);
    vTaskDelay(500 / portTICK_PERIOD_MS);  // update every 0.5 sec
  }
}

void rfidTask(void* parameter) {
  while (1) {
    if (rfid.PICC_IsNewCardPresent()) { // new tag is available
      if (rfid.PICC_ReadCardSerial()) { // NUID has been readed

        if (compareUID(rfid.uid.uidByte, rfid.uid.size)) {
          // Valid
          digitalWrite(GREEN_LED, 1);
          esp_timer_start_once(green_timer, TOGGLE_INTERVAL_GREEN);
          Serial.print("Valid card read at ");
          printTime();
        } else {
          // Invalid
          digitalWrite(RED_LED, 1);
          esp_timer_start_once(red_timer, TOGGLE_INTERVAL_RED);
          Serial.print("Invalid card read at ");
          printTime();
          Serial.print("     Invalid card ID: ");
          for (int i = 0; i < rfid.uid.size; i++) {
            Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
            Serial.print(rfid.uid.uidByte[i], HEX);
          }
          Serial.println();
        }

        vTaskDelay(3000 / portTICK_PERIOD_MS);

        rfid.PICC_HaltA(); // halt PICC
        rfid.PCD_StopCrypto1(); // stop encryption on PCD
      }
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);  // slight pause
  }
}

void setup() {
  // initialize serial port
  Serial.begin(115200);

  SPI.begin(36, 37, 35, SS_PIN);
  rfid.PCD_Init(); // init MFRC522

  // initialize button
  pinMode(button, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  if (! rtc.begin()) {
    Serial.println("RTC module is NOT found");
    Serial.flush();
    while (1);
  }
  // automatically sets the RTC to the date & time on PC this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)) + TimeSpan(0, 0, 0, 14));

  const esp_timer_create_args_t green_timer_args = {
    .callback = &greenOnTimer,
    .arg = NULL,
    .name = "green_timer"
  };

  const esp_timer_create_args_t red_timer_args = {
    .callback = &redOnTimer,
    .arg = NULL,
    .name = "red_timer"
  };

  esp_timer_create(&green_timer_args, &green_timer);
  esp_timer_create(&red_timer_args, &red_timer);

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");

  // initialize lcd
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.display();
  delay(2);

  // initialize temperature sensor
  tempSensor.setWire(&Wire);

  // initialize binary semaphore
  dataReady = xSemaphoreCreateBinary();
  buttonSemaphore = xSemaphoreCreateBinary();
  lcdReady = xSemaphoreCreateMutex();
  xSemaphoreGive(buttonSemaphore);
  xSemaphoreGive(lcdReady);

  xTaskCreatePinnedToCore(clockTask, "Clock Task", 4096, NULL, 0, &clockTaskHandle, 0);
  xTaskCreatePinnedToCore(rfidTask, "RFID Task", 8192, NULL, 1, &rfidTaskHandle, 1);
  // === TEMPERATURE SENSOR TASKS ===
  xTaskCreatePinnedToCore(TASK_SensorData, "sensor data", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(TASK_DisplayData, "Data Display", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TASK_ButtonHandler, "handle button press", 4096, NULL, 3, NULL, 0); 
  // === BUTTON INTERUPT FOR SENSOR DISPLAY ===
  attachInterrupt(digitalPinToInterrupt(button), handleButtonInterrupt, FALLING);

  delay(2000);
}

void loop() {}