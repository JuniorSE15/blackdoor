#include "storage.h"
#include "event.h"
#include "rfid.h"
#include "mqtt.h"
#include "esp_task_wdt.h"

void triggerRelay(int state) {
    digitalWrite(RELAY_PIN, state);
}

void handleRFIDEvent(void* pvParameters) {
    esp_task_wdt_add(NULL);
    if (isEnrollmentState) {
        Serial.println("Enrollment mode active. stopping RFID event handler.");
        vTaskDelay(pdMS_TO_TICKS(1000));
        return;
    }
        for (;;) {
            String cardUID = readRFIDCard();

            if (cardUID.length() > 0) {
                Serial.print("Found NFC tag with UID: ");
                Serial.println(cardUID);
                
                if (isCardAuthorized(cardUID)) {
                    Serial.println("Card authorized. Unlocking door.");
                    isOpen = true;
                    triggerRelay(LOW);
                } else {
                    Serial.println("Card not authorized.");
                }
            }
            esp_task_wdt_reset();
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    vTaskDelete(NULL);
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
        } else if (isEnrollmentState) {
            publishState("enrollment");
        } else {
            publishState("locked");
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
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


void handleEnrollmentEvent(void* pvParameters) {
    esp_task_wdt_add(NULL);
    for (;;) {
        if (isEnrollmentState) {
            Serial.println("Enrollment mode active. Waiting for new RFID card...");
            String newCardUID = readRFIDCard();
            if (newCardUID.length() > 0) {
                Serial.print("Enrolling new card with UID: ");
                Serial.println(newCardUID);
                // save it to nvs (eeprom)
                saveCardUID(newCardUID);
                isEnrollmentState = false;
                Serial.println("Enrollment complete. Exiting enrollment mode.");
            }
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    vTaskDelete(NULL);
}