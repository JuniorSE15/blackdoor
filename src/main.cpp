#include <Arduino.h>
#include <stdio.h>
#include <WiFi.h>

#include "config.h"
#include "event.h"
#include "rfid.h"
#include "mqtt.h"
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

  const char* wifi_ssid = getenv("WIFI_SSID");
  const char* wifi_password = getenv("WIFI_PASSWORD");
  const char* mqtt_broker = getenv("MQTT_BROKER");
  const int mqtt_port = 1883;
  const char* device_id = getenv("DEVICE_ID");

  if (connectToWiFi(wifi_ssid, wifi_password)) {
    setupMQTT(device_id, mqtt_broker, mqtt_port);
    connectToMQTT();
  }

  xTaskCreate(handleRFIDEvent, "RFID Event Handler", 4096, NULL, 1, NULL);
  xTaskCreate(handleDoorEvent, "Door Event Handler", 4096, NULL, 1, NULL);
  xTaskCreate(handleMQTTEvent, "MQTT Event Handler", 4096, NULL, 1, NULL);
}

void loop()
{
  Serial.print("Door State: ");
  Serial.println(isOpen ? "OPEN" : "CLOSED");
  delay(500);

  esp_task_wdt_reset();
}