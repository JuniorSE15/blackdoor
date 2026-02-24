#include "config.h"

#ifdef ARDUINO
#include <Arduino.h>
#else
// Stubs for native testing
#define OUTPUT 1
#define INPUT 0
#define INPUT_PULLUP 2
inline void pinMode(int, int) {}
#endif

void setupConfig() {
    pinMode(TOUCH_PIN, INPUT);

    pinMode(RELAY_PIN, OUTPUT);
    pinMode(RFID_DATA_PIN, INPUT);
    pinMode(RFID_CLK_PIN, INPUT);

    pinMode(REED_SWITCH_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(TOUCHPAD_PIN, INPUT);

    nfc.begin();
    nfc.SAMConfig();
}

void calibrateHardware() {
    // TODO: implement
}
