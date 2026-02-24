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