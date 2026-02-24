#pragma once
#include <Adafruit_PN532.h>

// This file is for project-wide configuration settings and macros.
#ifndef CONFIG_H
#define CONFIG_H

#define RELAY_PIN 26 // GPIO pin for locking mechanism
#define RFID_DATA_PIN 21 // GPIO pin for RFID sensor
#define RFID_CLK_PIN 22 // GPIO pin for RFID sensor

#define REED_SWITCH_PIN 5 // GPIO pin for reed switch
#define LED_PIN 13 
#define TOUCHPAD_PIN 12

// debug
#define TOUCH_PIN 12

#define DOOR_UNLOCK_TIME 8000 // 8 seconds to hold the door open

Adafruit_PN532 nfc(RFID_DATA_PIN, RFID_CLK_PIN);

void setupConfig();
void calibrateHardware();

#endif // CONFIG_H