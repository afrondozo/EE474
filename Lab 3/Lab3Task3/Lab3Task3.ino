#include <esp32-hal-timer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <BLEDevice.h>
#include <BLEServer.h>

#define SERVICE_UUID        "9bb6b113-953a-4247-abb1-8ff87f903b7d"
#define CHARACTERISTIC_UUID "16b23f91-5dfe-4e12-8c83-1d7537820822"

LiquidCrystal_I2C lcd(0x27, 16, 2);

hw_timer_t *timer = NULL;

// --- Define Timer Interval --- //
#define TOGGLE_INTERVAL_US 1000000  // 1 second

// --- Define Pins --- //
#define BUTTON 5

esp_timer_handle_t periodic_timer;

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200;  // milliseconds

// --- Define Flags --- //
bool buttonPressed = false;
bool showBLEMessage = false;
bool showTimer = true;
volatile uint32_t timerValue = 0;

void IRAM_ATTR onTimer() {
  timerValue++;
}

void IRAM_ATTR handleButtonInterrupt() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > debounceDelay) {
    lastInterruptTime = currentTime;
    showTimer = false;
    buttonPressed = true;
  }
}

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

  timer = timerBegin(1000000);
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0);

  BLEDevice::init("MyESP32"); // Change this to an unique name
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                       CHARACTERISTIC_UUID,
                                       BLECharacteristic::PROPERTY_READ |
                                       BLECharacteristic::PROPERTY_WRITE
                                     );
  pService->start();
  // the initial value for the characteristic we defined
  pCharacteristic->setValue("1");
  // set call back
  pCharacteristic->setCallbacks(new MyCallbacks());
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  // displayTimer();
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

void displayBLEMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("New message!");
  delay(2000);
}

void displayTimer() {
  lcd.setCursor(0, 0);
  lcd.print("Binary Counter:");

  lcd.setCursor(0, 1);
  lcd.print(toBinary(timerValue, 16)); // Show 8 bits

  delay(2);
}

void displayButtonPressed() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Button pressed!");

  delay(2);
}

// --- Convert number to binary string with leading zeros --- //
String toBinary(uint32_t num, int bits) {
  String s = "";
  for (int i = bits - 1; i >= 0; i--) {
    s += ((num >> i) & 1) ? '1' : '0';
  }
  return s;
}
