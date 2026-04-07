#include "event.h"
#include "rfid.h"
#include "mqtt.h"
#include "esp_task_wdt.h"

// ---------------------------------------------------------------------------
// Authorized NFC UIDs (lowercase hex, no spaces).
// Add the UID printed in the serial monitor when you scan your card.
// ---------------------------------------------------------------------------
static const char* AUTHORIZED_UIDS[] = {
    "aabbccdd",   // Card 1 — replace with your actual UID
    "11223344",   // Card 2 — replace with your actual UID
    nullptr       // sentinel — keep last
};

static bool isAuthorizedCard(const String& uid) {
    for (int i = 0; AUTHORIZED_UIDS[i] != nullptr; i++) {
        if (uid.equalsIgnoreCase(String(AUTHORIZED_UIDS[i]))) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------

void triggerRelay(int state) {
    digitalWrite(RELAY_PIN, state);
}

// ---------------------------------------------------------------------------
// RFID task — polls NFC reader, checks UID against allowed list
// ---------------------------------------------------------------------------
void handleRFIDEvent(void* pvParameters) {
    esp_task_wdt_add(NULL);
    unsigned long lastRead = millis();
    const unsigned long readInterval = 200;

    for (;;) {
        if (millis() - lastRead > readInterval) {
            lastRead = millis();
            String cardUID = readRFIDCard();

            if (cardUID.length() > 0) {
                Serial.print("[RFID] Card UID: ");
                Serial.println(cardUID);

                if (isAuthorizedCard(cardUID)) {
                    Serial.println("[RFID] Authorized — unlocking!");
                    if (!isOpen) {
                        isOpen = true;
                        triggerRelay(LOW);       // LOW = relay energised = unlocked
                        publishState("unlocked");
                    }
                } else {
                    Serial.println("[RFID] DENIED — unauthorized card.");
                }
            }
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(100));
        taskYIELD();
    }
}

// ---------------------------------------------------------------------------
// MQTT task — keeps connection alive and publishes heartbeat state
// ---------------------------------------------------------------------------
void handleMQTTEvent(void* pvParameters) {
    esp_task_wdt_add(NULL);

    unsigned long lastPublish = millis();
    const unsigned long publishInterval = 5000;

    for (;;) {
        if (!mqttClient.connected()) {
            connectToMQTT();
        }
        mqttClient.loop();

        if (millis() - lastPublish > publishInterval) {
            lastPublish = millis();
            publishState(isOpen ? "unlocked" : "locked");
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ---------------------------------------------------------------------------
// Door event task — reed switch debounce + non-blocking auto-lock timer.
//
// State logic:
//   isOpen=true  → relay LOW (unlocked). Timer starts.
//     Door physically opens  → timer resets (stays unlocked while open).
//     Door physically closes → timer resumes. After DOOR_UNLOCK_TIME → lock.
//   isOpen=false → relay HIGH (locked). Nothing to do.
// ---------------------------------------------------------------------------
void handleDoorEvent(void *pvParameters) {
    esp_task_wdt_add(NULL);

    // Debounce state
    bool lastRawOpen     = false;
    bool stableDoorOpen  = false;
    unsigned long debounceStartMs = 0;
    const unsigned long DEBOUNCE_MS = 50;

    // Auto-lock timer state
    bool prevIsOpen          = false;
    unsigned long unlockStartMs = 0;

    for (;;) {
        // --- Reed switch debounce -------------------------------------------
        // Reed switch: LOW when door is physically OPEN (magnet moves away).
        bool rawOpen = (digitalRead(REED_SWITCH_PIN) == LOW);
        if (rawOpen != lastRawOpen) {
            lastRawOpen    = rawOpen;
            debounceStartMs = millis();
        }
        if ((millis() - debounceStartMs) >= DEBOUNCE_MS) {
            stableDoorOpen = lastRawOpen;
        }

        // --- Auto-lock timer ------------------------------------------------
        if (isOpen && !prevIsOpen) {
            // Transition: just became unlocked — start the auto-lock countdown.
            unlockStartMs = millis();
            Serial.println("[DOOR] Unlocked — auto-lock timer started.");
        }
        prevIsOpen = isOpen;

        if (isOpen) {
            if (stableDoorOpen) {
                // Door is physically open — keep resetting the timer.
                unlockStartMs = millis();
                Serial.println("[DOOR] Door physically open.");
            } else {
                // Door is closed but relay still unlocked — check timeout.
                if ((millis() - unlockStartMs) >= DOOR_UNLOCK_TIME) {
                    isOpen = false;
                    triggerRelay(HIGH);       // HIGH = relay off = locked
                    publishState("locked");
                    Serial.println("[DOOR] Auto-locked after timeout.");
                }
            }
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(200));
        taskYIELD();
    }

    vTaskDelete(NULL);
}