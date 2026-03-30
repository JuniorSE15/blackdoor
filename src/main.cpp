#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "event.h"
#include "rfid.h"

volatile bool isOpen = false; // Global state variable to track if the door is currently open

void setup()
{
  Serial.begin(115200);
  delay(1000);

  setupConfig();
  calibrateHardware();

  // Initialize RFID/NFC reader
  setupRFID();

  xTaskCreate(handleRFIDEvent, "RFID Event Handler", 4096, NULL, 1, NULL);
  xTaskCreate(handleDoorEvent, "Door Event Handler", 4096, NULL, 1, NULL);
}

void loop()
{
  // Check for RFID card
  String cardUID = readRFIDCard();

  if (cardUID.length() > 0)
  {
    // Card detected - you can add your authorization logic here
    Serial.print("Card UID detected: ");
    Serial.println(cardUID);

    // Example: unlock door if card is authorized
    if (!isOpen)
    {
      isOpen = true;
      digitalWrite(RELAY_PIN, HIGH);
      Serial.println("RFID card authorized — door unlocked!");
    }
    delay(1500);
  }

  // Check touch sensor
  touch_value_t touchValue = touchRead(TOUCH_PIN);
  Serial.print("Touch Value: ");
  Serial.println(touchValue);

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
}