#pragma once

#include "config.h"

#ifndef EVENT_H
#define EVENT_H

extern volatile bool isOpen;
extern volatile bool isEnrollmentState;

void triggerRelay(int state);
void saveCardUID(const String& uid);
bool isCardAuthorized(const String& uid);

void handleRFIDEvent(void* pvParameters);
void handleDoorEvent(void* pvParameters);
void handleMQTTEvent(void* pvParameters);
void handleEnrollmentEvent(void* pvParameters);

#endif // EVENT_H