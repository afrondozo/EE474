int state = 1;
int inputPin = 40;
int reading;
int previous;

unsigned long lastToggleTime = 0;           // the last time the output pin was toggled
unsigned long debounce = 200UL;   // the debounce time, increase if the output flickers

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(19, OUTPUT);
  pinMode(21, OUTPUT);
  pinMode(inputPin, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  reading = digitalRead(inputPin);

  if (reading == HIGH && previous == LOW && millis() - lastToggleTime > debounce) {
    if (state) {
      state = 0;
    } else {
      state = 1;
    }

    lastToggleTime = millis();
  }

  blink(state);

  previous = reading;
  
}

void blink(int state) {
  if (state) {
    digitalWrite(19, HIGH);
    delay(500);
    digitalWrite(19, LOW);
    delay(500);
  } else {
    digitalWrite(19, LOW);
    delay(1000);
  }
}