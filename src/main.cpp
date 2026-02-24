#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "event.h"

volatile bool isOpen = false; // Global state variable to track if the door is currently open

void setup() {
  Serial.begin(115200);

  setupConfig();
  calibrateHardware();

  xTaskCreate(handleRFIDEvent, "RFID Event Handler", 4096, NULL, 1, NULL);
  xTaskCreate(handleDoorEvent, "Door Event Handler", 4096, NULL, 1, NULL);
}

void loop() {
  touch_value_t touchValue = touchRead(TOUCH_PIN);
  Serial.print("Touch Value: ");
  Serial.println(touchValue);

  if (touchValue < 40) {
    if (!isOpen) {
      isOpen = true;
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Touch detected — door unlocked!");
    }
  }
  delay(100);
}