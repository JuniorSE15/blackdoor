#include "event.h"
#include "rfid.h"
#include "mqtt.h"
#include "esp_task_wdt.h"

void triggerRelay(int state) {
    digitalWrite(RELAY_PIN, state);
}

void handleRFIDEvent(void* pvParameters) {
    esp_task_wdt_add(NULL);
    unsigned long lastRead = millis();
    const unsigned long readInterval = 200;  // Read RFID every 200ms max
    
    for (;;) {
        // Only poll RFID every 200ms to reduce I2C bus contention
        if (millis() - lastRead > readInterval) {
            lastRead = millis();
            String cardUID = readRFIDCard();

            if (cardUID.length() > 0) {
                Serial.print("[RFID] Found NFC tag with UID: ");
                Serial.println(cardUID);
                // TODO: Implement logic to check if the cardUID is authorized to unlock the door
                isOpen = true;
                triggerRelay(LOW);
            }
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(100));
        taskYIELD();  // Explicitly yield to other tasks
    }
}

void handleMQTTEvent(void* pvParameters) {
    esp_task_wdt_add(NULL);
    
    unsigned long lastPublish = millis();
    const unsigned long publishInterval = 5000;  // Publish every 5s instead of every 100ms
    
    for (;;) {
        if (!mqttClient.connected()) {
            connectToMQTT();
        }
        mqttClient.loop();

        // Publish door state every 5 seconds (not every 100ms)
        if (millis() - lastPublish > publishInterval) {
            lastPublish = millis();
            if (isOpen) {
                publishState("unlocked");
            } else {
                publishState("locked");
            }
        }
        
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(500));  // Check MQTT more frequently but publish less often
    }
}

void handleDoorEvent(void *pvParameters) {
    esp_task_wdt_add(NULL);
    for (;;) {
        bool doorPhysicallyOpen = digitalRead(REED_SWITCH_PIN) == LOW;

        if (isOpen) {
            if (doorPhysicallyOpen) {
                Serial.println("Door is open.");
            } else {
                vTaskDelay(pdMS_TO_TICKS(DOOR_UNLOCK_TIME));
                isOpen = false;
                triggerRelay(HIGH);
                Serial.println("Door auto-locked.");
            }
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(200));
        taskYIELD();  // Explicitly yield to lower-priority tasks
    }

    vTaskDelete(NULL);
}