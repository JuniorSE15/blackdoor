#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "event.h"
#include "rfid.h"
#include "reed.h"

volatile bool isOpen = false;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  setupConfig();
  // HIGH means relay is off (door locked), LOW means relay is on (door unlocked)
  wakeUpHardware(&isOpen);

  // Initialize RFID/NFC reader
  setupRFID();

  xTaskCreate(handleRFIDEvent, "RFID Event Handler", 4096, NULL, 1, NULL);
  xTaskCreate(handleDoorEvent, "Door Event Handler", 4096, NULL, 1, NULL);

  reed_init(18);
  reed_set_callback(onReedChange);
}

void loop()
{
  reed_update();
  // Check touch sensor
  touch_value_t touchValue = touchRead(TOUCH_PIN);
  Serial.print("Touch Value: ");
  Serial.println(touchValue);

  Serial.print("Door State: ");
  Serial.println(isOpen ? "OPEN" : "CLOSED");

  if (touchValue < 40)
  {
    if (!isOpen)
    {
      isOpen = true;
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("Touch detected — door unlocked!");
    }
  }

  if (reed_is_open())
  {
    Serial.println("The door is open from reed");
  }
  else
  {
    Serial.println("The door is closed from reed");
  }

  delay(500);
}