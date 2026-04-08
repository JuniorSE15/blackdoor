#pragma once
#include <Adafruit_PN532.h>

// This file is for project-wide configuration settings and macros.
#ifndef CONFIG_H
#define CONFIG_H

#define RELAY_PIN 26 // GPIO pin for locking mechanism

#define RFID_DATA_PIN 21 // GPIO pin for RFID sensor
#define RFID_CLK_PIN 22  // GPIO pin for RFID sensor

#define KEYPAD_CLK_PIN 18  // TTP229 SCL/clock pin
#define KEYPAD_DATA_PIN 19 // TTP229 SDO/data pin

#define REED_SWITCH_PIN 15 // GPIO pin for reed switch
#define LED_PIN 13
#define TOUCHPAD_PIN 12
#define BUZZER_PIN 14

#define BUTTON_PIN 12 

#define DOOR_UNLOCK_TIME 8000 // 8 seconds to hold the door open

typedef struct {
    const char* username;
    const char* password;
} Mqttparams_t;

extern Adafruit_PN532 nfc; // Declare as extern (defined in rfid.cpp)

void setupConfig();
void wakeUpHardware(volatile bool *isOpen);
void triggerRelay(int state); // LOW = unlock, HIGH = lock
void buzz(int frequency, int duration); // Function to control the buzzer

#endif // CONFIG_H