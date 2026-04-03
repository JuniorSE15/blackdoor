#include <Arduino.h>
#include <stdio.h>
#include <Wire.h> 

#include "config.h"
#include "event.h"
#include "rfid.h"

// I2C settings for Stella Slave
#define I2C_SLAVE_ADDR 0x08
#define STELLA_SDA_PIN 32  // SDA Blue wire 
#define STELLA_SCL_PIN 33  // SCL Yellow wire 

volatile bool isOpen = false; 
volatile bool stellaUnlockRequested = false;

// I2C receive event handler for Wire1 (Stella Slave)
void receiveEvent(int howMany) {
    while (Wire1.available()) {
        char command = Wire1.read();
        if (command == 0x01) {
            stellaUnlockRequested = true;
        }
    }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  setupConfig();
  wakeUpHardware(&isOpen);

  // setupRFID();

  Wire1.begin(I2C_SLAVE_ADDR, STELLA_SDA_PIN, STELLA_SCL_PIN, 100000);
  Wire1.onReceive(receiveEvent);

  // xTaskCreate(handleRFIDEvent, "RFID Event Handler", 4096, NULL, 1, NULL);
  xTaskCreate(handleDoorEvent, "Door Event Handler", 4096, NULL, 1, NULL);
  
  Serial.println("Blackdoor System Ready.");
  Serial.println("RFID Master on Wire (21/22) | Stella Slave on Wire1 (32/33)");
}

void loop()
{
  touch_value_t touchValue = touchRead(TOUCH_PIN);

  if (touchValue < 40)
  {
    if (!isOpen)
    {
      isOpen = true;
      digitalWrite(RELAY_PIN, HIGH); 
      Serial.println("[HARDWARE] Touch detected — door unlocked!");
    }
  }

  // Stella UWB Logic
  if (stellaUnlockRequested) 
  {
    stellaUnlockRequested = false; 
    
    if (!isOpen)
    {
      isOpen = true;
      digitalWrite(RELAY_PIN, HIGH); 
      Serial.println("[STELLA] UWB threshold met — door unlocked!");
    }
  }

  delay(500);
}