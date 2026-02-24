#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "event.h"

void setup() {
  Serial.begin(115200);

  setupConfig();
  calibrateHardware();

  xTaskCreate(handleRFIDEvent, "RFID Event Handler", 4096, NULL, 1, NULL);
}

void loop() {
  touch_value_t touchValue = touchRead(TOUCH_PIN);
  Serial.print("Touch Value: ");
  Serial.println(touchValue);

  if (touchValue < 40) {
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("Relay Activated!");
  } else {
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Relay Deactivated!");
  }
  delay(100);
}