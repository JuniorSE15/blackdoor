#include <Arduino.h>
#include <stdio.h>

#include "config.h"

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(TOUCH_PIN, INPUT);
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
}