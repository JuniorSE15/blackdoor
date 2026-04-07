#pragma once

#include "config.h"

#ifndef EVENT_H
#define EVENT_H

// Task entry points — each runs as a FreeRTOS task.
void handleRFIDEvent(void* pvParameters);
void handleDoorEvent(void* pvParameters);
void handleMQTTEvent(void* pvParameters);
void handleKeypadEvent(void* pvParameters);

// isOpen    → extern volatile bool isOpen  (defined in access_control.cpp)
// triggerRelay() → declared in config.h   (defined in config.cpp)

#endif // EVENT_H
