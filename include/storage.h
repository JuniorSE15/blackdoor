#ifndef STORAGE_H
#define STORAGE_H
#include <Preferences.h>

Preferences cardStorage;

const char* NVS_NAMESPACE = "cards";
const int MAX_CARDS = 20;

void saveCardUID(const String& uid);
bool isCardAuthorized(const String& uid);

#endif // STORAGE_H