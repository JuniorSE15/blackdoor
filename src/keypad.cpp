#include "keypad.h"
#include "config.h"

namespace {
const int KEY_COUNT = 8;
const char KEY_MAP[9] = {
    'X',
    '1', '2', '3', 'A',
    '4', '5', '6', 'B',
};

int lastKey = 0;
}

void setupKeypad()
{
    pinMode(KEYPAD_CLK_PIN, OUTPUT);
    pinMode(KEYPAD_DATA_PIN, INPUT_PULLUP);
    digitalWrite(KEYPAD_CLK_PIN, HIGH);
}

uint16_t readRawKeypad()
{
    uint16_t data = 0;

    for (int i = 0; i < KEY_COUNT; i++)
    {
        digitalWrite(KEYPAD_CLK_PIN, LOW);
        delayMicroseconds(50);

        int bit = digitalRead(KEYPAD_DATA_PIN);
        data |= (bit << i);

        digitalWrite(KEYPAD_CLK_PIN, HIGH);
        delayMicroseconds(50);
    }

    return data;
}

int decodeKeyFromRaw(uint16_t data)
{
    for (int i = 0; i < KEY_COUNT; i++)
    {
        if (((data >> i) & 1) == 0)
        {
            return i + 1;
        }
    }

    return 0;
}

char pollKeypadChar()
{
    const uint16_t raw = readRawKeypad();
    const int key = decodeKeyFromRaw(raw);

    if (key != 0 && key != lastKey)
    {
        lastKey = key;
        return KEY_MAP[key];
    }

    if (key == 0)
    {
        lastKey = 0;
    }

    return '\0';
}
