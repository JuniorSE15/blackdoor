#include "event.h"
#include "rfid.h"

void triggerRelay(int state) {
    digitalWrite(RELAY_PIN, state);
}

void handleRFIDEvent(void* pvParameters) {
    for (;;) {
        String cardUID = readRFIDCard();

        if (cardUID.length() > 0) {
            Serial.print("Found NFC tag with UID: ");
            Serial.println(cardUID);
            // TODO: Implement logic to handle the card
            isOpen = true;
            triggerRelay(LOW);
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Add delay to yield to other tasks
    }
}

void handleDoorEvent(void *pvParameters) {
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
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    vTaskDelete(NULL);
}