#include <Arduino.h>
#include <stdio.h>
#include <Wire.h> 
#include <WiFi.h>

#include "config.h"
#include "event.h"
#include "rfid.h"
#include "mqtt.h"
#include "esp_task_wdt.h"

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

  // const char* wifi_ssid = getenv("WIFI_SSID");
  // const char* wifi_password = getenv("WIFI_PASSWORD");
  // const char* mqtt_broker = getenv("MQTT_BROKER");
  // const int mqtt_port = 1883;
  // const char* device_id = getenv("DEVICE_ID");

  // if (connectToWiFi(wifi_ssid, wifi_password)) {
  //   setupMQTT(device_id, mqtt_broker, mqtt_port);
  //   connectToMQTT();
  // }

  xTaskCreate(handleRFIDEvent, "RFID Event Handler", 4096, NULL, 1, NULL);
  xTaskCreate(handleDoorEvent, "Door Event Handler", 4096, NULL, 1, NULL);
  // xTaskCreate(handleMQTTEvent, "MQTT Event Handler", 4096, NULL, 1, NULL);
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
  Serial.print("Door State: ");
  Serial.println(isOpen ? "OPEN" : "CLOSED");
  delay(500);

  esp_task_wdt_reset();
}
#endif