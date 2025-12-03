// Filename: 474FinalProject.ino
// Authors: Kylie Neal, Aidan Frondozo
// Description: implements a smart rfid security system

// === INCLUDES ===
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "LiquidCrystal_I2C.h"
#include "Wire.h"
#include "AM2320_asukiaaa.h"
#include <SPI.h>
#include <MFRC522.h>
#include <RTClib.h>

// === DEFINES ===
#define SS_PIN  5  
#define RST_PIN 11 
#define RED_LED 7
#define GREEN_LED 6
#define TOGGLE_INTERVAL_GREEN 2000000  // 2 seconds
#define TOGGLE_INTERVAL_RED 2000000  // 2 seconds

// === RFID ===
MFRC522 rfid(SS_PIN, RST_PIN);
RTC_DS3231 rtc;
DateTime now;
const byte correctUID[] = {0xAA, 0x22, 0x0A, 0x01};
const byte correctUIDLength = 4;

// === TIMER HANDLES ===
esp_timer_handle_t green_timer;
esp_timer_handle_t red_timer;

/**
 * @brief ISR callback to turn off the green LED
 *
 * This interrupt service routine is triggered by a hardware timer
 * and sets the GREEN_LED pin low, turning the LED off.
 *
 * @param arg unused parameter
 */
void IRAM_ATTR greenOnTimer(void* arg) {
  digitalWrite(GREEN_LED, 0);
}
/**
 * @brief ISR callback to turn off the red LED
 *
 * This interrupt service routine is triggered by a hardware timer
 * and sets the RED_LED pin low, turning the LED off.
 *
 * @param arg unused parameter
 */
void IRAM_ATTR redOnTimer(void* arg) {
  digitalWrite(RED_LED, 0);
}

// === DAYS OF WEEK ===
char daysOfWeek[7][12] = {
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

// === TASK HANDLES ===
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
/**
 * @brief Compares a scanned RFID UID with the correct UID
 *
 * Checks whether the given UID matches the stored correct UID.
 * Returns true only if the lengths match and every byte is identical.
 *
 * @param uid pointer to the scanned UID array
 * @param uidLength length of the scanned UID in bytes
 * @return true if the UID matches the expected UID, false otherwise
 */
bool compareUID(byte *uid, byte uidLength) {
  if (uidLength != correctUIDLength) return false;
  for (int i = 0; i < uidLength; i++) {
    if (uid[i] != correctUID[i]) return false;
  }
  return true;
}
/**
 * @brief Prints the current date and time to the serial monitor
 *
 * Formats and outputs the current timestamp including year, month,
 * day, weekday, hour, minute, and second. Assumes a valid RTC
 * object providing the 'now' DateTime.
 */
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
/**
 * @brief FreeRTOS task to continuously update the LCD with the current time.
 * 
 * This task retrieves the current time from the RTC and updates the top row
 * of the LCD every 500 ms. The LCD is accessed using the `lcdReady` semaphore
 * to avoid concurrent writes from other tasks. The displayed format is
 * HH:MM:SS, padded to fit the LCD width.
 * 
 * @param parameter Pointer to task parameters (not used).
 * 
 * @note Runs indefinitely in a while(1) loop.
 * @note Uses the global RTC object `rtc`, the global DateTime `now`,
 *       and the semaphore `lcdReady`.
 */
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
/**
 * @brief FreeRTOS task to handle RFID card scanning and authentication.
 * 
 * This task continuously checks for new RFID cards using the MFRC522 reader.
 * When a new UID is detected, it verifies the UID using `compareUID()`.
 * Depending on whether the card is valid or invalid, it triggers the
 * corresponding LED indicator and logs a timestamped message to Serial.
 * 
 * A correct UID activates the green LED; an incorrect UID activates the red LED.
 * Both are controlled by one-shot timers (`green_timer` and `red_timer`).
 * 
 * @param parameter Pointer to task parameters (not used).
 * 
 * @note Uses global objects `rfid`, `green_timer`, `red_timer`,
 *       and helper function `printTime()`.
 * @note Calls `compareUID()` to validate scanned cards.
 * @note Includes small delays to reduce polling rate and avoid rapid retriggers.
 */
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

  // initialize rfid communication
  SPI.begin(36, 37, 35, SS_PIN);
  rfid.PCD_Init(); // init MFRC522

  // initialize button
  pinMode(button, INPUT_PULLUP);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // initialize clock
  if (! rtc.begin()) {
    Serial.println("RTC module is NOT found");
    Serial.flush();
    while (1);
  }
  // automatically sets the RTC to the date & time on PC this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)) + TimeSpan(0, 0, 0, 14));

  // initialize timer arguments
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

  // initialize timers
  esp_timer_create(&green_timer_args, &green_timer);
  esp_timer_create(&red_timer_args, &red_timer);

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

  // === CLOCK AND RFID TASKS ===
  xTaskCreatePinnedToCore(clockTask, "Clock Task", 4096, NULL, 0, &clockTaskHandle, 0);
  xTaskCreatePinnedToCore(rfidTask, "RFID Task", 8192, NULL, 1, &rfidTaskHandle, 1);
  // === TEMPERATURE SENSOR TASKS ===
  xTaskCreatePinnedToCore(TASK_SensorData, "sensor data", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(TASK_DisplayData, "Data Display", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TASK_ButtonHandler, "handle button press", 4096, NULL, 3, NULL, 0); 
  // === BUTTON INTERUPT FOR SENSOR DISPLAY ===
  attachInterrupt(digitalPinToInterrupt(button), handleButtonInterrupt, FALLING);

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  delay(2000);
}

void loop() {}