#include <Arduino.h>
#include <stdio.h>
#include <Wire.h>
#include <WiFi.h>

#include "config.h"
#include "event.h"
#include "access_control.h"
#include "rfid.h"
#include "keypad.h"
#include "mqtt.h"
#include "esp_task_wdt.h"

// I2C settings for Stella UWB slave
#define I2C_SLAVE_ADDR 0x08
#define STELLA_SDA_PIN 32
#define STELLA_SCL_PIN 33

#define WDT_TIMEOUT_SEC 10

#ifndef PIO_UNIT_TESTING
// isOpen is defined in access_control.cpp; extern declared in access_control.h

volatile bool stellaUnlockRequested = false;

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

    // esp_task_wdt_init(WDT_TIMEOUT_SEC, true);
    // esp_task_wdt_add(NULL);

    setupConfig();
    setupAccessControl();   // Loads NVS cards + password, locks door.
    setupKeypad();

    Wire1.begin(I2C_SLAVE_ADDR, STELLA_SDA_PIN, STELLA_SCL_PIN, 100000);
    Wire1.onReceive(receiveEvent);
    Serial.println("Blackdoor System Ready.");
    Serial.println("RFID Master on Wire (21/22) | Stella Slave on Wire1 (32/33)");

    setupRFID();

    if (connectToWiFi(WIFI_SSID, WIFI_PASSWORD)) {
        setupMQTT(DEVICE_ID, MQTT_BROKER, 1883);
        connectToMQTT(MQTT_USER, MQTT_PASSWORD);
    }

    static Mqttparams_t myArgs = {MQTT_USER, MQTT_PASSWORD};

    // Task priorities: higher number = higher priority.
    xTaskCreate(handleDoorEvent,   "Door Event Handler",   4096, NULL, 3, NULL);
    xTaskCreate(handleRFIDEvent,   "RFID Event Handler",   4096, NULL, 2, NULL);
    xTaskCreate(handleKeypadEvent, "Keypad Event Handler", 4096, NULL, 1, NULL);
    // parse in username and password
    xTaskCreate(handleMQTTEvent,   "MQTT Event Handler",   4096, (void *)&myArgs, 1, NULL);
}

void loop()
{
    // -- Button test (GPIO 12) --
    if (digitalRead(BUTTON_PIN) == HIGH) {
        Serial.println("[BUTTON] Press detected — toggling lock state.");
        if (getLockState() == LockState::LOCKED) {
            grantAccess(AccessSource::TOUCH);
            publishState("unlocked");
        } else {
            triggerRelay(HIGH); // Lock the door
            publishState("locked");
        }
        delay(500); // Debounce delay for button
    }

    // ── Stella UWB ───────────────────────────────────────────────────────
    if (stellaUnlockRequested) {
        stellaUnlockRequested = false;
        if (getLockState() == LockState::LOCKED) {
            Serial.println("[STELLA] UWB threshold met — unlocking!");
            grantAccess(AccessSource::UWB);
            publishState("unlocked");
        }
    }

    // esp_task_wdt_reset();
    delay(500);
}
#endif
