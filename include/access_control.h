#pragma once
#include <Arduino.h>

// ─── State machine states ────────────────────────────────────────────────────
enum class LockState {
    LOCKED,               // Default: relay de-energised, door locked
    UNLOCKED,             // Relay energised, auto-lock timer running
    ADMIN_MODE,           // Master card was scanned — next scan adds/revokes a card
    PASSWORD_CHANGE_MODE  // 3-step keypad flow: old pwd → new pwd → confirm
};

// ─── Source that triggered an unlock ────────────────────────────────────────
enum class AccessSource {
    RFID,
    KEYPAD,
    TOUCH,
    UWB
};

// ─── Shared volatile lock state (defined in access_control.cpp) ─────────────
extern volatile bool isOpen;

// ─── Lifecycle ───────────────────────────────────────────────────────────────
// Call once in setup() after setupConfig().
void setupAccessControl();

// Call regularly from a high-priority task (handleDoorEvent).
// Drives the auto-lock timer and mode timeouts.
void handleStateMachine();

// ─── Core lock control ───────────────────────────────────────────────────────
void      grantAccess(AccessSource source);  // Unlock + start auto-lock timer
void      lockDoor();                        // Lock immediately
LockState getLockState();

// Reset auto-lock timer (call while door is physically open so it doesn't
// auto-lock while someone is walking through).
void resetUnlockTimer();

// ─── Mode transitions ────────────────────────────────────────────────────────
void enterAdminMode();
void exitAdminMode();           // Reverts to LOCKED
void enterPasswordChangeMode();
void exitPasswordChangeMode();  // Reverts to LOCKED

// ─── RFID card database (NVS-backed, survives power cycles) ─────────────────
bool isMasterCard(const String& uid);       // Compare against hardcoded master UID
bool isAuthorizedCard(const String& uid);   // Check NVS card list
bool addCard(const String& uid);            // Returns false if full or duplicate
bool revokeCard(const String& uid);         // Returns false if not found

// ─── Password management (NVS-backed) ───────────────────────────────────────
bool verifyPassword(const String& input);
bool changePassword(const String& oldPwd, const String& newPwd); // min 4 digits
void forceSetPassword(const String& newPwd); // From iOS app
