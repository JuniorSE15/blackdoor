#include "config.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

// Define the nfc object here (declared as extern in config.h)
Adafruit_PN532 nfc(RFID_DATA_PIN, RFID_CLK_PIN);

void setupConfig()
{
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(RFID_DATA_PIN, INPUT);
    pinMode(RFID_CLK_PIN, INPUT);

    pinMode(REED_SWITCH_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(TOUCHPAD_PIN, INPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    nfc.begin();
    nfc.SAMConfig();
}

void buzz(int frequency, int duration) {
    tone(BUZZER_PIN, frequency, duration);
    delay(duration); // Wait for the tone to finish
    noTone(BUZZER_PIN); // Stop the tone
}

void triggerRelay(int state) {
    digitalWrite(RELAY_PIN, state);
}

void wakeUpHardware(volatile bool *isOpen)
{
    triggerRelay(HIGH);  // Ensure door starts locked
    *isOpen = false;
}
