// Authors: Aidan Frondozo, Kylie Neal
// Date: 11/1/25
// Filename: Lab3Task1.ino
// Description: This task takes a serial input and prints the message on the LCD.


#include<Wire.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

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
    int bytesRead = Serial.readBytesUntil('\n', message, sizeof(message) - 1);
    message[bytesRead] = '\0';
    sendCommand(0x80);
    delay(2);
    sendCommand(0x01);
    delay(2);
    for (int i = 0; i < bytesRead; i++) {
      sendData(message[i]);
    }
  }
}

void sendData(uint8_t data) {
  uint8_t high = data & 0xF0;
  uint8_t low = data << 4;
  Wire.beginTransmission(ADDR);
  Wire.write(high | 0x0D);  // bit 3 = backlight on
  Wire.write((high & ~0x04) | 0x09);  // bit 3 = backlight on
  Wire.write(low | 0x0D);  // bit 3 = backlight on
  Wire.write((low & ~0x04) | 0x09);  // bit 3 = backlight on
  Wire.endTransmission();
}

void sendCommand(uint8_t data) {
  int8_t high = data & 0xF0;
  uint8_t low = data << 4;
  Wire.beginTransmission(ADDR);
  Wire.write(high | 0x0C);  // bit 3 = backlight on
  Wire.write((high & ~0x04) | 0x08);  // bit 3 = backlight on
  Wire.write(low | 0x0C);  // bit 3 = backlight on
  Wire.write((low & ~0x04) | 0x08);  // bit 3 = backlight on
  Wire.endTransmission();
}

