#pragma once
// This file is for project-wide configuration settings and macros.
#ifndef CONFIG_H
#define CONFIG_H

#define LOCK_PIN 2 // GPIO pin for locking mechanism
#define RFID_SENSOR_PIN 4 // GPIO pin for RFID sensor
#define REED_SWITCH_PIN 5 // GPIO pin for reed switch
#define LED_PIN 13 // GPIO pin for status LED
#define TOUCHPAD_PIN 12 // GPIO pin for touchpad

void setupConfig(); 
void calibrateHardware();

#endif // CONFIG_H