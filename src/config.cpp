#include "config.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

// Define the nfc object here (declared as extern in config.h)
Adafruit_PN532 nfc(RFID_DATA_PIN, RFID_CLK_PIN);

void setupConfig()
{
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

void wakeUpHardware(volatile bool *isOpen)
{ 
    // TODO: implement
    digitalWrite(RELAY_PIN, HIGH);
    *isOpen = false;
}
