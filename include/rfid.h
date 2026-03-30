#ifndef RFID_H
#define RFID_H

#include <Arduino.h>

// Initialize the PN532 NFC reader
void setupRFID();

// Check for NFC card and return UID as a string
// Returns empty string if no card detected
String readRFIDCard();

// Check if a card is present (non-blocking)
bool isCardPresent();

#endif
