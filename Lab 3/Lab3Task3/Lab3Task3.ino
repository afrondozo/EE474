// Filename: Lab3Task3.ino
// Authors: Aidan Frondozo, Kylie Neal
// Date: 11/5/25
// Description: This file implements using interrupts and the esp32 BLE module to recieve button or
// bluetooth inputs and display messages on an LCD screen.

#include <esp32-hal-timer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BLEDevice.h>
#include <BLEServer.h>

// ===== DEFINE BLE MODULES UUIDS =====
#define SERVICE_UUID        "9bb6b113-953a-4247-abb1-8ff87f903b7d"
#define CHARACTERISTIC_UUID "16b23f91-5dfe-4e12-8c83-1d7537820822"
// ===== INITIALIZE LCD =====
LiquidCrystal_I2C lcd(0x27, 16, 2);
// ===== DEFINE BUTTON PIN =====
#define BUTTON 5
// ===== DEFINE TIMER =====
hw_timer_t *timer = NULL;

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;  // milliseconds

// ===== DEFINE FLAGS =====
bool buttonPressed = false;
bool showBLEMessage = false;
bool showTimer = true;
volatile uint32_t timerValue = 0;

// Name: onTimer
// Description: increments the timer value
void IRAM_ATTR onTimer() {
  timerValue++;
}

// Name: handleButtonInterrupt
// Description: sets button flags and handles the debounce delay for button presses
void IRAM_ATTR handleButtonInterrupt() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > debounceDelay) {
    lastInterruptTime = currentTime;
    showTimer = false;
    buttonPressed = true;
  }
}

// Name: onWrite
// Description: sets the BLE flag when a new value is written via BLE
class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() > 0) {
      showBLEMessage = true;
      showTimer = false;
    }
  }
};

void setup() {
  Serial.begin(115200);
  delay(1000);
  lcd.init();
  lcd.backlight();

  pinMode(BUTTON, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON), handleButtonInterrupt, FALLING);

  // Initialize timer
  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0);
  // Initialize bluetooth
  BLEDevice::init("MyESP32"); // Change this to an unique name
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                       CHARACTERISTIC_UUID,
                                       BLECharacteristic::PROPERTY_READ |
                                       BLECharacteristic::PROPERTY_WRITE
                                    );
  pService->start();
  pCharacteristic->setValue("1");
  pCharacteristic->setCallbacks(new MyCallbacks()); // set callback function
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  if (buttonPressed) {
    Serial.println("Button Pressed!");
    timerStop(timer);
    displayButtonPressed();
    delay(2000);
    timerStart(timer);
    showTimer = true;
    buttonPressed = false;
    lcd.clear();
  } else if (showTimer) {
    displayTimer();
  } else if (showBLEMessage) {
    showBLEMessage = false;
    showTimer = true;
    timerStop(timer);
    displayBLEMessage();
    timerStart(timer);
    lcd.clear();
  }
}

// Name: displayBLEMessage
// Description: displays BLE message on the LCD
void displayBLEMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New message!");
  delay(2000);
}
// Name: displayTimer
// Description: displays the timer count on the LCD
void displayTimer() {
  lcd.setCursor(0, 0);
  lcd.print("Binary Counter:");
  lcd.setCursor(0, 1);
  lcd.print(toBinary(timerValue, 16)); // Show 16 bits
  delay(2);
}
// Name: displayButtonPressed
// Description: displays the button pressed message on the LCD
void displayButtonPressed() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Button pressed!");
  delay(2);
}

// Name: toBinary
// Description: converts a 32 bit binary number into a string
String toBinary(uint32_t num, int bits) {
  String s = "";
  for (int i = bits - 1; i >= 0; i--) {
    s += ((num >> i) & 1) ? '1' : '0';
  }
  return s;
}
