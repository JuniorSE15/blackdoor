#include <storage.h>

void saveCardUID(const String& uid) {
    cardStorage.begin(NVS_NAMESPACE, false);
    
    int cardCount = cardStorage.getInt("count", 0);
    
    // Check if card already exists
    for (int i = 0; i < cardCount; i++) {
        String key = "card" + String(i);
        String existingUID = cardStorage.getString(key.c_str(), "");
        if (existingUID == uid) {
            Serial.println("Card already enrolled.");
            cardStorage.end();
            return;
        }
    }
    
    if (cardCount >= MAX_CARDS) {
        Serial.println("Max cards reached. Cannot enroll more.");
        cardStorage.end();
        return;
    }
    
    String key = "card" + String(cardCount);
    cardStorage.putString(key.c_str(), uid);
    cardStorage.putInt("count", cardCount + 1);
    
    Serial.print("Card saved at slot ");
    Serial.println(cardCount);
    
    cardStorage.end();
}

bool isCardAuthorized(const String& uid) {
    cardStorage.begin(NVS_NAMESPACE, true);  // read-only
    
    int cardCount = cardStorage.getInt("count", 0);
    
    for (int i = 0; i < cardCount; i++) {
        String key = "card" + String(i);
        String storedUID = cardStorage.getString(key.c_str(), "");
        if (storedUID == uid) {
            cardStorage.end();
            return true;
        }
    }
    
    cardStorage.end();
    return false;
}