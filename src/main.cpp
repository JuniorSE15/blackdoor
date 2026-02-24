#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "event.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  setupConfig();
  calibrateHardware();
}

void loop() {
  // put your main code here, to run repeatedly:
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

  xTaskHandle_t eventTaskHandle;
  xTaskCreate(handleRFIDEvent, "RFID Event Task", 2048, NULL, 1, &eventTaskHandle);
}