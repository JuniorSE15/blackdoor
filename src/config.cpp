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
    pinMode(LOCK_PIN, OUTPUT);
    pinMode(RFID_SENSOR_PIN, INPUT);
    pinMode(REED_SWITCH_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    pinMode(TOUCHPAD_PIN, INPUT);
}

void calibrateHardware() {
    // TODO: implement
}
