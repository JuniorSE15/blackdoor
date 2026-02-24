#pragma once

#include "config.h"

#ifndef EVENT_H
#define EVENT_H

extern volatile bool isOpen;

void handleRFIDEvent(void* pvParameters);
void handleDoorEvent(void* pvParameters);

#endif // EVENT_H