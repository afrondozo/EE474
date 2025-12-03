// Filename: 474FinalProject.ino
// Authors: Kylie Neal, Aidan Frondozo
// Description: implements a smart rfid security system

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "LiquidCrystal_I2C.h"
#include "Wire.h"
#include "AM2320_asukiaaa.h" 

// === LCD ===
LiquidCrystal_I2C lcd0(0x27, 16, 2);

// === SENSOR SEMAPHORE ===
SemaphoreHandle_t dataReady;

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
  lcd0.clear();
  lcd0.print("Temperature:");
  lcd0.setCursor(0, 1);
  lcd0.print(String(temp) + "F");
}
/**
* @brief Helper method for outputting a humidity display on LCD
*
* Displays current outdoor humidity reading on LCD
* @param humid current humidity
*/
void displayHumidity (float humid) {
  lcd0.clear();
  lcd0.print("Humidity:");
  lcd0.setCursor(0, 1);
  lcd0.print(String(humid) + "%");
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
      if (buttonState) {
        displayTemp(currTemp);
      } else {
        displayHumidity(humidity);
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
          Serial.println(buttonState ? "Temperature" : "Humidity");
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


void setup() {
  // initialize serial port
  Serial.begin(115200);

  // initialize button
  pinMode(button, INPUT_PULLUP);

  // initialize lcd0
  Wire.begin();
  lcd0.init();
  lcd0.backlight();
  lcd0.display();
  delay(2);

  // initialize temperature sensor
  tempSensor.setWire(&Wire);

  // initialize binary semaphore
  dataReady = xSemaphoreCreateBinary();
  buttonSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(buttonSemaphore);

  // === TEMPERATURE SENSOR TASKS ===
  xTaskCreatePinnedToCore(TASK_SensorData, "sensor data", 4096, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(TASK_DisplayData, "Data Display", 4096, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(TASK_ButtonHandler, "handle button press", 4096, NULL, 3, NULL, 0); 
  // === BUTTON INTERUPT FOR SENSOR DISPLAY ===
  attachInterrupt(digitalPinToInterrupt(button), handleButtonInterrupt, FALLING);

  delay(2000);
  Serial.println("started");
}

void loop() {}