#pragma once

#ifndef KEYPAD_H
#define KEYPAD_H

#include <Arduino.h>

void setupKeypad();
uint16_t readRawKeypad();
int decodeKeyFromRaw(uint16_t data);
char pollKeypadChar();

#endif // KEYPAD_H
