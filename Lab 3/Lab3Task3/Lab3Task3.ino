#include<esp32-hal-timer.h>
#include<Wire.h>
#include<LiquidCrystal_I2C.h>

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
volatile uint32_t timerValue = 0;

void IRAM_ATTR onTimer() {
  timerValue++;
}

void IRAM_ATTR handleButtonInterrupt() {
  buttonPressed = true;
}

void displayTimer() {
  lcd.setCursor(0, 0);
  lcd.print("Binary Counter:");

  lcd.setCursor(0, 1);
  lcd.print(toBinary(timerValue, 16)); // Show 8 bits

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
}

void loop() {
  displayTimer();
  if (buttonPressed) {
    Serial.println("Button Pressed!");
    delay(2000);
    buttonPressed = false;
  }
}
