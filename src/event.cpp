#include "event.h"
#include "access_control.h"
#include "rfid.h"
#include "keypad.h"
#include "mqtt.h"
#include "esp_task_wdt.h"

// ─────────────────────────────────────────────────────────────────────────────
// RFID Event Task  (Priority 2)
//
// State-aware NFC card handling:
//   LOCKED      — authorize card → unlock; master card → admin mode
//   ADMIN_MODE  — next card scanned: add if new, revoke if known;
//                 re-scan master to exit
//   Others      — ignore RFID input
//
// Card detection:
//   - readRFIDCard() only returns a UID when a NEW card is detected
//   - Holding the card will not trigger multiple reads
// ─────────────────────────────────────────────────────────────────────────────
void handleRFIDEvent(void *pvParameters)
{
    esp_task_wdt_add(NULL);

    unsigned long lastRead = millis();
    const unsigned long READ_MS = 200;

    for (;;)
    {
        if (millis() - lastRead >= READ_MS)
        {
            lastRead = millis();
            String uid = readRFIDCard();

            // readRFIDCard() only returns non-empty string for NEW card detections
            if (uid.length() > 0)
            {
                LockState st = getLockState();

                if (isMasterCard(uid))
                {
                    if (st == LockState::ADMIN_MODE)
                    {
                        Serial.println("[RFID] Master card — exiting admin mode.");
                        exitAdminMode();
                    }
                    else
                    {
                        Serial.println("[RFID] Master card — entering admin mode.");
                        enterAdminMode();
                    }
                }
                else if (st == LockState::ADMIN_MODE)
                {
                    if (isAuthorizedCard(uid))
                    {
                        Serial.println("[RFID] Known card — revoking.");
                        revokeCard(uid);
                        publishCards(); // sync updated list to backend DB
                    }
                    else
                    {
                        Serial.println("[RFID] New card — adding.");
                        addCard(uid);
                        buzz(1000, 100); // Beep on card add/revoke
                        publishCards(); // sync updated list to backend DB
                    }
                    if (isRemoteEnrollMode())
                    {
                        Serial.println("[RFID] Remote enrollment complete — exiting admin mode.");
                        exitAdminMode();
                    }
                }
                else if (st == LockState::LOCKED)
                {
                    if (isAuthorizedCard(uid))
                    {
                        Serial.println("[RFID] Authorized — unlocking!");
                        grantAccess(AccessSource::RFID);
                        publishState("unlocked");
                    }
                    else
                    {
                        Serial.println("[RFID] DENIED — unauthorized card.");
                        buzz(500, 500);
                    }
                }
                // Ignore RFID when UNLOCKED or PASSWORD_CHANGE_MODE.
            }
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(100));
        taskYIELD();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Keypad Event Task  (Priority 1)
//
// Key layout (TTP229 8-key):  1  2  3  A  4  5  6  B
//   A = ENTER / SUBMIT
//   B = CLEAR buffer  |  cancel password change  |  trigger change mode
//
// Flows:
//   LOCKED:
//     Digits → buffer.  A → verify password → grantAccess if correct.
//     B → clear buffer.
//
//   UNLOCKED:
//     B → enter PASSWORD_CHANGE_MODE (user already authenticated).
//
//   PASSWORD_CHANGE_MODE (3-step):
//     Step 1 ENTER_OLD:    enter old pwd + A → verify
//     Step 2 ENTER_NEW:    enter new pwd + A → store (min 4 digits)
//     Step 3 CONFIRM_NEW:  re-enter new pwd + A → match → save → LOCKED
//     B at any step → cancel → LOCKED
// ─────────────────────────────────────────────────────────────────────────────
#define MAX_INPUT 8

static char inputBuf[MAX_INPUT + 1];
static int inputLen = 0;

enum class KeypadSubState
{
    IDLE,
    ENTER_OLD,
    ENTER_NEW,
    CONFIRM_NEW
};
static KeypadSubState kSubState = KeypadSubState::IDLE;
static String pwdOld = "";
static String pwdNew = "";

static void clearInput()
{
    inputLen = 0;
    inputBuf[0] = '\0';
}

static void appendInput(char c)
{
    if (inputLen < MAX_INPUT)
    {
        inputBuf[inputLen++] = c;
        inputBuf[inputLen] = '\0';
    }
}

void handleKeypadEvent(void *pvParameters)
{
    esp_task_wdt_add(NULL);

    for (;;)
    {
        // If the system left PASSWORD_CHANGE_MODE (e.g. timeout), reset sub-state.
        if (getLockState() != LockState::PASSWORD_CHANGE_MODE &&
            kSubState != KeypadSubState::IDLE)
        {
            kSubState = KeypadSubState::IDLE;
            pwdOld = "";
            pwdNew = "";
            clearInput();
        }

        char key = pollKeypadChar();

        if (key != '\0')
        {
            LockState st = getLockState();

            if (key == 'B')
            {
                // ── CLEAR / CANCEL ──────────────────────────────────────────
                if (st == LockState::UNLOCKED)
                {
                    // Enter password change mode from unlocked state.
                    clearInput();
                    kSubState = KeypadSubState::ENTER_OLD;
                    enterPasswordChangeMode();
                    Serial.println("[KEYPAD] PWD CHANGE — enter OLD password + A");
                }
                else if (st == LockState::PASSWORD_CHANGE_MODE)
                {
                    clearInput();
                    kSubState = KeypadSubState::IDLE;
                    pwdOld = "";
                    pwdNew = "";
                    exitPasswordChangeMode();
                    Serial.println("[KEYPAD] Password change cancelled.");
                }
                else
                {
                    // LOCKED: just clear input buffer.
                    clearInput();
                    Serial.println("[KEYPAD] Buffer cleared.");
                }
            }
            else if (key == 'A')
            {
                // ── SUBMIT ──────────────────────────────────────────────────
                String input = String(inputBuf);
                clearInput();

                if (st == LockState::LOCKED)
                {
                    if (verifyPassword(input))
                    {
                        Serial.println("[KEYPAD] Correct password — unlocking!");
                        grantAccess(AccessSource::KEYPAD);
                        publishState("unlocked");
                    }
                    else
                    {
                        Serial.println("[KEYPAD] WRONG password.");
                        buzz(500, 500);
                    }
                }
                else if (st == LockState::PASSWORD_CHANGE_MODE)
                {
                    switch (kSubState)
                    {
                    case KeypadSubState::ENTER_OLD:
                        if (verifyPassword(input))
                        {
                            pwdOld = input;
                            kSubState = KeypadSubState::ENTER_NEW;
                            Serial.println("[KEYPAD] Old password OK — enter NEW password + A (min 4 digits)");
                        }
                        else
                        {
                            Serial.println("[KEYPAD] Wrong old password — try again.");
                        }
                        break;

                    case KeypadSubState::ENTER_NEW:
                        if (input.length() >= 4)
                        {
                            pwdNew = input;
                            kSubState = KeypadSubState::CONFIRM_NEW;
                            Serial.println("[KEYPAD] New password set — confirm it + A");
                        }
                        else
                        {
                            Serial.println("[KEYPAD] Too short (min 4 digits) — enter NEW password + A");
                        }
                        break;

                    case KeypadSubState::CONFIRM_NEW:
                        if (input == pwdNew)
                        {
                            changePassword(pwdOld, pwdNew);
                            kSubState = KeypadSubState::IDLE;
                            pwdOld = "";
                            pwdNew = "";
                            exitPasswordChangeMode(); // Locks door.
                            Serial.println("[KEYPAD] Password saved. Door locked.");
                        }
                        else
                        {
                            kSubState = KeypadSubState::ENTER_NEW;
                            Serial.println("[KEYPAD] Passwords don't match — re-enter NEW password + A");
                        }
                        break;

                    default:
                        break;
                    }
                }
            }
            else if (key >= '1' && key <= '6')
            {
                // ── DIGIT ────────────────────────────────────────────────────
                if (st == LockState::LOCKED || st == LockState::PASSWORD_CHANGE_MODE)
                {
                    appendInput(key);
                    Serial.printf("[KEYPAD] %d digit(s) entered.\n", inputLen);
                }
            }
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(40));
    }

    vTaskDelete(NULL);
}

// ─────────────────────────────────────────────────────────────────────────────
// MQTT Event Task  (Priority 1)
// Maintains MQTT connection and publishes a heartbeat every 5 s.
// ─────────────────────────────────────────────────────────────────────────────
void handleMQTTEvent(void *pvParameters)
{
    // esp_task_wdt_add(NULL);

    unsigned long lastPublish = millis();
    const unsigned long PUB_MS = 1000;

    for (;;)
    {
        if (!mqttClient.connected())
        {
            connectToMQTT(MQTT_USER, MQTT_PASSWORD);
        }
        mqttClient.loop();

        if (millis() - lastPublish >= PUB_MS)
        {
            lastPublish = millis();
            publishState(isOpen ? "unlocked" : "locked");
        }

        // esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Door Event Task  (Priority 3 — highest)
//
// Responsibilities:
//   1. Debounce reed switch (door physically open/closed).
//   2. While door is physically open and state is UNLOCKED, reset auto-lock
//      timer so the door doesn't lock while someone walks through.
//   3. Tick the state machine (auto-lock timer, mode timeouts).
// ─────────────────────────────────────────────────────────────────────────────
void handleDoorEvent(void *pvParameters)
{
    esp_task_wdt_add(NULL);

    bool lastRawOpen = false;
    bool stableDoorOpen = false;
    bool prevStableDoorOpen = false; // last state we acted on — for change detection
    unsigned long debounceStartMs = 0;
    const unsigned long DEBOUNCE_MS = 50;

    for (;;)
    {
        // Reed switch wiring: GPIO 15 INPUT_PULLUP, NC switch.
        //   Magnet present (door CLOSED) → NC contacts open → pull-up holds HIGH → LOW=false
        //   Magnet absent  (door OPEN)   → NC contacts close → GPIO pulled LOW  → LOW=true
        bool rawOpen = (digitalRead(REED_SWITCH_PIN) == HIGH);

        // Debounce: ignore readings until the raw value has been stable for
        // DEBOUNCE_MS. Eliminates switch bounce and door-frame vibration noise.
        if (rawOpen != lastRawOpen)
        {
            lastRawOpen = rawOpen;
            debounceStartMs = millis();
        }
        if ((millis() - debounceStartMs) >= DEBOUNCE_MS)
        {
            stableDoorOpen = lastRawOpen;
        }

        // Act only on state CHANGES to avoid flooding MQTT and Serial.
        if (stableDoorOpen != prevStableDoorOpen)
        {
            prevStableDoorOpen = stableDoorOpen;
            doorPhysicallyOpen = stableDoorOpen; // update shared flag

            if (stableDoorOpen)
            {
                Serial.println("[DOOR] Reed: door OPENED (magnet gone, contacts closed, GPIO LOW).");
                publishState("open");
            }
            else
            {
                Serial.println("[DOOR] Reed: door CLOSED (magnet present, contacts open, GPIO HIGH).");
                // Door is back in frame — report the actual solenoid state.
                publishState(isOpen ? "unlocked" : "locked");
            }
        }

        // While door is physically open and solenoid is unlocked, keep resetting
        // the auto-lock timer so the bolt doesn't try to extend while the door
        // is still moving through the frame.
        if (stableDoorOpen && getLockState() == LockState::UNLOCKED)
        {
            resetUnlockTimer();
        }

        // Conflict: if the state machine timed out and just locked the solenoid
        // while the door is still physically open, warn on Serial. The lock still
        // executes (security takes priority), but MQTT will have published "open"
        // when the door opened so the app already knows.
        if (stableDoorOpen && getLockState() == LockState::LOCKED)
        {
            Serial.println("[DOOR] WARNING: solenoid LOCKED while door is physically open.");
        }

        // Tick the state machine (auto-lock, admin/pwd-change timeouts).
        handleStateMachine();

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(200));
        taskYIELD();
    }

    vTaskDelete(NULL);
}
