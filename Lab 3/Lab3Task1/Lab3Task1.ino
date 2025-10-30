#include<Wire.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define ADDR 0x27

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  delay(2);

}

void loop() {
  char buf[32];
  if (Serial.available()) {
    int len = Serial.readBytesUntil('\n', buf, sizeof(buf) - 1);
    buf[len] = '\0';  // null-terminate it
    send(0x01, 0);
    delay(2);
    send(0x80, 0);
    for (int i = 0; i < len; i++) {
      send(buf[i], 1);
    }
  }
}

// --- Low-level LCD helpers --- //
void expanderWrite(uint8_t data) {
  Wire.beginTransmission(ADDR);
  Wire.write(data | 0x08);  // bit 3 = backlight on
  Wire.endTransmission();
}

void pulseEnable(uint8_t data) {
  expanderWrite(data | 0x04);   // EN=1
  delayMicroseconds(1);
  expanderWrite(data & ~0x04);  // EN=0
  delayMicroseconds(50);
}

void write4bits(uint8_t value) {
  expanderWrite(value);
  pulseEnable(value);
}

void send(uint8_t value, uint8_t mode) {
  // mode=0 for command, 1 for data
  uint8_t high = value & 0xF0;
  uint8_t low  = (value << 4) & 0xF0;
  write4bits(high | mode);
  write4bits(low | mode);
}

void sendArray(const uint8_t *str) {
  while (str) {
    send(*str, 1);
    *str++;
  }
}
