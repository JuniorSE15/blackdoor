#include <Arduino.h>
#include <stdio.h>

#include "config.h"
#include "event.h"
#include "rfid.h"
#include "mqtt.h"
#include "esp_task_wdt.h"

#define WDT_TIMEOUT_SEC 10

#ifndef WIFI_SSID
#define WIFI_SSID "blackdoor-ap"
#endif
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "changeme"
#endif
#ifndef MQTT_BROKER
#define MQTT_BROKER "10.69.0.22"
#endif
#ifndef MQTT_PORT
#define MQTT_PORT 1883
#endif
#ifndef DEVICE_ID
#define DEVICE_ID "door1"
#endif

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

  if (connectToWiFi(WIFI_SSID, WIFI_PASSWORD)) {
    setupMQTT(DEVICE_ID, MQTT_BROKER, MQTT_PORT);
    connectToMQTT();
  }

  xTaskCreate(handleRFIDEvent, "RFID Event Handler", 4096, NULL, 1, NULL);
  xTaskCreate(handleDoorEvent, "Door Event Handler", 4096, NULL, 1, NULL);
  xTaskCreate(handleMQTTEvent, "MQTT Event Handler", 4096, NULL, 1, NULL);
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