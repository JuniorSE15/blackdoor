#include "rfid.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_PN532.h>

// I2C pins for ESP32
#define SDA_PIN 21
#define SCL_PIN 22

void setupRFID()
{
    Serial.println("=================================");
    Serial.println("   ESP32 + PN532 (I2C MODE)      ");
    Serial.println("=================================");

    Wire.begin(SDA_PIN, SCL_PIN);
    delay(1000);

    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();

    if (!versiondata)
    {
        Serial.println("[+] PN532 not found (I2C)");
        Serial.println("[+] Check wiring & switch mode");
        while (1)
            ;
    }

    Serial.println("[+] PN532 detected!");
    Serial.print("Firmware: ");
    Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print(".");
    Serial.println((versiondata >> 8) & 0xFF, DEC);

    nfc.SAMConfig();

    Serial.println("[+] Waiting for NFC card...");
    Serial.println("---------------------------------");
}

String readRFIDCard()
{
    uint8_t uid[7];
    uint8_t uidLength;

    bool success = nfc.readPassiveTargetID(
        PN532_MIFARE_ISO14443A,
        uid,
        &uidLength,
        100 
    );

    if (success)
    {
        Serial.println("📱 NFC Card Detected!");

        // Build UID string
        String uidString = "";
        Serial.print("UID: ");
        for (uint8_t i = 0; i < uidLength; i++)
        {
            if (uid[i] < 0x10)
            {
                uidString += "0";
                Serial.print("0");
            }
            uidString += String(uid[i], HEX);

            Serial.print("0x");
            if (uid[i] < 0x10)
                Serial.print("0");
            Serial.print(uid[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
        Serial.println("---------------------------------");

        return uidString;
    }

    return ""; // No card detected
}

bool isCardPresent()
{
    uint8_t uid[7];
    uint8_t uidLength;

    return nfc.readPassiveTargetID(
        PN532_MIFARE_ISO14443A,
        uid,
        &uidLength,
        100 // Timeout in ms
    );
}
