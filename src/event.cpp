#include "event.h"
#include "rfid.h"
#include "mqtt.h"
#include "esp_task_wdt.h"

void triggerRelay(int state) {
    digitalWrite(RELAY_PIN, state);
}

void handleRFIDEvent(void* pvParameters) {
    esp_task_wdt_add(NULL);
    for (;;) {
        String cardUID = readRFIDCard();

        if (cardUID.length() > 0) {
            Serial.print("Found NFC tag with UID: ");
            Serial.println(cardUID);
            // TODO: Implement logic to check if the cardUID is authorized to unlock the door
            isOpen = true;
            triggerRelay(LOW);
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void handleMQTTEvent(void* pvParameters) {
    esp_task_wdt_add(NULL);
    
    for (;;) {
        if (!mqttClient.connected()) {
            connectToMQTT();
        }
        mqttClient.loop();

        // publish door state periodically
        if (isOpen) {
            publishState("unlocked");
        } else {
            publishState("locked");
        }
        
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(100));
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
    }

    vTaskDelete(NULL);
}