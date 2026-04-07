#include "access_control.h"
#include "config.h"
#include <Preferences.h>

// ─── Configuration ───────────────────────────────────────────────────────────
// Set MASTER_CARD_UID to the UID printed when you scan your master card.
static const char MASTER_CARD_UID[] = "d948ca06";
static const char DEFAULT_PASSWORD[] = "1234";

// DOOR_UNLOCK_TIME comes from config.h (8 s). Override there if needed.
static const uint32_t UNLOCK_MS = DOOR_UNLOCK_TIME;
static const uint32_t ADMIN_MS = 30000;   // Admin mode timeout: 30 s
static const uint32_t PWD_CHG_MS = 60000; // Password change timeout: 60 s
static const int MAX_CARDS = 20;

// ─── Shared state ────────────────────────────────────────────────────────────
volatile bool isOpen = false;

static LockState currentState = LockState::LOCKED;
static uint32_t unlockStartMs = 0;
static uint32_t modeStartMs = 0;

// ─── In-memory RFID card list ────────────────────────────────────────────────
static String cardList[MAX_CARDS];
static int cardCount = 0;

static Preferences prefs;

// ─── NVS helpers ─────────────────────────────────────────────────────────────
// Cards are stored as a semicolon-separated string: "uid1;uid2;uid3"
static void loadCards()
{
    prefs.begin("blackdoor", true);
    String raw = prefs.getString("cards", "");
    prefs.end();

    cardCount = 0;
    if (raw.isEmpty())
        return;

    int start = 0;
    int sep = raw.indexOf(';');
    while (sep != -1 && cardCount < MAX_CARDS)
    {
        cardList[cardCount++] = raw.substring(start, sep);
        start = sep + 1;
        sep = raw.indexOf(';', start);
    }
    if (start < (int)raw.length() && cardCount < MAX_CARDS)
    {
        cardList[cardCount++] = raw.substring(start);
    }
}

static void saveCards()
{
    String raw = "";
    for (int i = 0; i < cardCount; i++)
    {
        if (i > 0)
            raw += ";";
        raw += cardList[i];
    }
    prefs.begin("blackdoor", false);
    prefs.putString("cards", raw);
    prefs.end();
}

// ─── Setup ───────────────────────────────────────────────────────────────────
void setupAccessControl()
{
    // Ensure a default password exists in NVS on first boot.
    prefs.begin("blackdoor", false);
    if (!prefs.isKey("pwd"))
    {
        prefs.putString("pwd", DEFAULT_PASSWORD);
        Serial.printf("[AC] First boot — default password set to \"%s\"\n", DEFAULT_PASSWORD);
    }
    prefs.end();

    loadCards();
    lockDoor(); // Always start locked.
    Serial.printf("[AC] Ready. %d card(s) in database.\n", cardCount);
    Serial.printf("[AC] Master card UID: %s\n", MASTER_CARD_UID);
}

// ─── Core lock control ───────────────────────────────────────────────────────
void grantAccess(AccessSource source)
{
    static const char *names[] = {"RFID", "KEYPAD", "TOUCH", "UWB"};
    Serial.printf("[AC] Access GRANTED via %s\n", names[static_cast<int>(source)]);

    isOpen = true;
    unlockStartMs = millis();
    currentState = LockState::UNLOCKED;
    triggerRelay(LOW); // LOW = relay energised = unlocked
}

void lockDoor()
{
    isOpen = false;
    currentState = LockState::LOCKED;
    triggerRelay(HIGH); // HIGH = relay off = locked
    Serial.println("[AC] Door LOCKED");
}

LockState getLockState()
{
    return currentState;
}

void resetUnlockTimer()
{
    if (currentState == LockState::UNLOCKED)
    {
        unlockStartMs = millis();
    }
}

// ─── State machine tick ──────────────────────────────────────────────────────
// Call from a regularly-scheduled task (e.g., handleDoorEvent every 200 ms).
void handleStateMachine()
{
    uint32_t now = millis();

    switch (currentState)
    {
    case LockState::UNLOCKED:
        if (now - unlockStartMs >= UNLOCK_MS)
        {
            Serial.println("[AC] Auto-lock timeout.");
            lockDoor();
        }
        break;

    case LockState::ADMIN_MODE:
        if (now - modeStartMs >= ADMIN_MS)
        {
            Serial.println("[AC] Admin mode timed out — locking.");
            exitAdminMode();
        }
        break;

    case LockState::PASSWORD_CHANGE_MODE:
        if (now - modeStartMs >= PWD_CHG_MS)
        {
            Serial.println("[AC] Password change timed out — locking.");
            exitPasswordChangeMode();
        }
        break;

    case LockState::LOCKED:
    default:
        break;
    }
}

// ─── Mode transitions ────────────────────────────────────────────────────────
void enterAdminMode()
{
    currentState = LockState::ADMIN_MODE;
    modeStartMs = millis();
    Serial.println("[AC] *** ADMIN MODE ***");
    Serial.println("[AC] Scan card to ADD (if new) or REVOKE (if known).");
    Serial.println("[AC] Scan master card again to exit. Timeout: 30 s.");
}

void exitAdminMode()
{
    lockDoor();
}

void enterPasswordChangeMode()
{
    currentState = LockState::PASSWORD_CHANGE_MODE;
    modeStartMs = millis();
    Serial.println("[AC] *** PASSWORD CHANGE MODE ***");
    Serial.println("[AC] Enter old password + A, then new password + A, then confirm + A.");
    Serial.println("[AC] Press B to cancel. Timeout: 60 s.");
}

void exitPasswordChangeMode()
{
    lockDoor();
}

// ─── RFID card database ──────────────────────────────────────────────────────
bool isMasterCard(const String &uid)
{
    return uid.equalsIgnoreCase(String(MASTER_CARD_UID));
}

bool isAuthorizedCard(const String &uid)
{
    for (int i = 0; i < cardCount; i++)
    {
        if (uid.equalsIgnoreCase(cardList[i]))
            return true;
    }
    return false;
}

bool addCard(const String &uid)
{
    if (cardCount >= MAX_CARDS)
    {
        Serial.println("[AC] Card database full — cannot add.");
        return false;
    }
    if (isAuthorizedCard(uid))
    {
        Serial.printf("[AC] Card already authorized: %s\n", uid.c_str());
        return false;
    }
    cardList[cardCount++] = uid;
    saveCards();
    Serial.printf("[AC] Card ADDED: %s  (%d/%d)\n", uid.c_str(), cardCount, MAX_CARDS);
    return true;
}

bool revokeCard(const String &uid)
{
    for (int i = 0; i < cardCount; i++)
    {
        if (uid.equalsIgnoreCase(cardList[i]))
        {
            cardList[i] = cardList[--cardCount]; // swap with last, shrink list
            saveCards();
            Serial.printf("[AC] Card REVOKED: %s\n", uid.c_str());
            return true;
        }
    }
    Serial.printf("[AC] Card not found: %s\n", uid.c_str());
    return false;
}

// ─── Password management ─────────────────────────────────────────────────────
bool verifyPassword(const String &input)
{
    prefs.begin("blackdoor", true);
    String stored = prefs.getString("pwd", DEFAULT_PASSWORD);
    prefs.end();
    return input == stored;
}

bool changePassword(const String &oldPwd, const String &newPwd)
{
    if (!verifyPassword(oldPwd))
    {
        Serial.println("[AC] changePassword: old password incorrect.");
        return false;
    }
    if (newPwd.length() < 4)
    {
        Serial.println("[AC] changePassword: new password too short (min 4).");
        return false;
    }
    prefs.begin("blackdoor", false);
    prefs.putString("pwd", newPwd);
    prefs.end();
    Serial.println("[AC] Password CHANGED.");
    return true;
}
