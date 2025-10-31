#include<Wire.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define ADDR 0x27

void setup() {
  Serial.begin(115200);
  lcd.init();
  Wire.begin();
  delay(2);
}

void loop() {
  // count();
  countAlphabet();
}

void countAlphabet() {
  for (int i = 0; i < 16; i++) {
    writeLCD('A' + i);
    delay(1000);
  }
  lcd.setCursor(0, 1);
  delay(2);
  for(int i = 16; i < 26; i++) {
    writeLCD('A' + i);
    delay(1000);
  }
  sendCommand(0x01);
  sendCommand(0x80);
  delay(500);
}

void count() {
  for (int i = 1; i < 10; i++) {
    writeLCD('0' + i);
    delay(1000);
  }
  writeLCD('1');
  writeLCD('0');
  delay(1000);
  sendCommand(0x01);
  sendCommand(0x80);
  delay(500);
}

void writeLCD(uint8_t data) {
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

