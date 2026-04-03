#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "event.h"
#include "rfid.h"
#include "esp_task_wdt.h"

#define WDT_TIMEOUT_SEC 10

volatile bool isOpen = false; 

void setup()
{
  Serial.begin(115200);
  delay(1000);

  esp_task_wdt_init(WDT_TIMEOUT_SEC, true);
  esp_task_wdt_add(NULL);

  setupConfig();
  // HIGH means relay is off (door locked), LOW means relay is on (door unlocked)
  wakeUpHardware(&isOpen);

  setupRFID();

  xTaskCreate(handleRFIDEvent, "RFID Event Handler", 4096, NULL, 1, NULL);
  xTaskCreate(handleDoorEvent, "Door Event Handler", 4096, NULL, 1, NULL);
}

void loop()
{
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
  delay(500);
  esp_task_wdt_reset();
}