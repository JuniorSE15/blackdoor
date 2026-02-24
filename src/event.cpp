#include "event.h"

void handleRFIDEvent(void* pvParameters) {
    for (;;) {
        uint8_t uid[7];
        uint8_t uidLength;

        if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
            Serial.print("Found NFC tag with UID: ");
            for (uint8_t i = 0; i < uidLength; i++) {
                Serial.print(uid[i], HEX);
                Serial.print(" ");
            }
            Serial.println();
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    vTaskDelete(NULL);
}

void handleDoorEvent(void *pvParameters) {
    for (;;) {
        bool doorPhysicallyOpen = digitalRead(REED_SWITCH_PIN) == HIGH;

        if (isOpen) {
            if (doorPhysicallyOpen) {
                Serial.println("Door is open.");
            } else {
                vTaskDelay(pdMS_TO_TICKS(DOOR_UNLOCK_TIME));
                isOpen = false;
                digitalWrite(RELAY_PIN, LOW);
                Serial.println("Door auto-locked.");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    vTaskDelete(NULL);
}