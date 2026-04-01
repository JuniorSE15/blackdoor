#pragma once

#include "config.h"

#ifndef EVENT_H
#define EVENT_H

extern volatile bool isOpen;

void triggerRelay(int state);

void handleRFIDEvent(void *pvParameters);
void handleDoorEvent(void *pvParameters);

void onReedChange(bool);

#endif // EVENT_H