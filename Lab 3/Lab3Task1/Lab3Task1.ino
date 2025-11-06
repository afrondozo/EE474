// Authors: Aidan Frondozo, Kylie Neal
// Date: 11/1/25
// Filename: Lab3Task1.ino
// Description: This task takes a serial input and prints the message on the LCD.

// ===== INCLUDES ===== //
#include<Wire.h>
#include<LiquidCrystal_I2C.h>
// ===== INITIALIZE LCD ===== //
LiquidCrystal_I2C lcd(0x27, 16, 2);
// ===== DEFINE LCD ADDR ===== //
#define ADDR 0x27

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  delay(10);
  sendCommand(0x0C); // turns on display
}

void loop() {
  char message[32];
  if (Serial.available()) {
    int bytesRead = Serial.readBytesUntil('\n', message, sizeof(message) - 1); // get serial input
    message[bytesRead] = '\0';
    sendCommand(0x80); // set cursor to beginning of first line
    delay(2);
    sendCommand(0x01); // clears LCD screen
    delay(2);
    for (int i = 0; i < bytesRead; i++) {
      sendData(message[i]); // prints characters to LCD screen
    }
  }
}

// Name: sendData
// Description: sends data to the LCD screen by pulsing enable
void sendData(uint8_t data) {
  uint8_t high = data & 0xF0;
  uint8_t low = data << 4;
  Wire.beginTransmission(ADDR);
  Wire.write(high | 0x0D);  
  Wire.write((high & ~0x04) | 0x09);  
  Wire.write(low | 0x0D);  
  Wire.write((low & ~0x04) | 0x09);
  Wire.endTransmission();
}

// Name: sendCommand
// Description: sends command to the LCD screen by pulsing enable
void sendCommand(uint8_t data) {
  int8_t high = data & 0xF0;
  uint8_t low = data << 4;
  Wire.beginTransmission(ADDR);
  Wire.write(high | 0x0C);  
  Wire.write((high & ~0x04) | 0x08);  
  Wire.write(low | 0x0C);  
  Wire.write((low & ~0x04) | 0x08);  
  Wire.endTransmission();
}

