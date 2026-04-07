#include <Arduino.h>
#include <stdio.h>
#include <Wire.h>
#include <WiFi.h>

#include "config.h"
#include "event.h"
#include "rfid.h"
#include "mqtt.h"
#include "esp_task_wdt.h"

// Fallback defaults — override in platformio.ini build_flags
#ifndef WIFI_SSID
  #define WIFI_SSID "your_wifi_ssid"
#endif
#ifndef WIFI_PASSWORD
  #define WIFI_PASSWORD "your_wifi_password"
#endif
#ifndef MQTT_BROKER
  #define MQTT_BROKER "192.168.1.100"
#endif
#ifndef DEVICE_ID
  #define DEVICE_ID "door-001"
#endif

// I2C settings for Stella Slave
#define I2C_SLAVE_ADDR 0x08
#define STELLA_SDA_PIN 32  // SDA Blue wire 
#define STELLA_SCL_PIN 33  // SCL Yellow wire 

#define WDT_TIMEOUT_SEC 10

#ifndef PIO_UNIT_TESTING
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

  esp_task_wdt_init(WDT_TIMEOUT_SEC, true);
  esp_task_wdt_add(NULL);

  setupConfig();
  wakeUpHardware(&isOpen);

  Wire1.begin(I2C_SLAVE_ADDR, STELLA_SDA_PIN, STELLA_SCL_PIN, 100000);
  Wire1.onReceive(receiveEvent);
  Serial.println("Blackdoor System Ready.");
  Serial.println("RFID Master on Wire (21/22) | Stella Slave on Wire1 (32/33)");
  
  setupRFID();

  if (connectToWiFi(WIFI_SSID, WIFI_PASSWORD)) {
    setupMQTT(DEVICE_ID, MQTT_BROKER, 1883);
    connectToMQTT();
  }

  // Task priorities: higher number = higher priority; MQTT lowest (network I/O can be slow)
  xTaskCreate(handleDoorEvent, "Door Event Handler", 4096, NULL, 3, NULL);      // Highest: door safety
  xTaskCreate(handleRFIDEvent, "RFID Event Handler", 4096, NULL, 2, NULL);      // Medium: sensor input
  xTaskCreate(handleMQTTEvent, "MQTT Event Handler", 4096, NULL, 1, NULL);      // Lowest: network I/O
}

void loop()
{
  touch_value_t touchValue = touchRead(TOUCH_PIN);

  if (touchValue < 40)
  {
    if (!isOpen)
    {
      isOpen = true;
      triggerRelay(LOW);  // LOW = relay energised = door unlocked
      publishState("unlocked");
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
      triggerRelay(LOW);  // LOW = relay energised = door unlocked
      publishState("unlocked");
      Serial.println("[STELLA] UWB threshold met — door unlocked!");
    }
  }
  Serial.print("Door State: ");
  Serial.println(isOpen ? "UNLOCKED" : "LOCKED");
  delay(500);

  esp_task_wdt_reset();
}
#endif