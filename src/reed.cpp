#include "reed.h"
#include <Arduino.h>

static uint8_t reed_pin = 18;
static bool current_open = false;
static bool last_read = false;
static unsigned long last_debounce = 0;
static const unsigned long debounce_ms = 50;
static void (*change_cb)(bool) = nullptr;

void reed_init(uint8_t pin)
{
    reed_pin = pin;
    pinMode(reed_pin, INPUT_PULLUP);
    last_read = digitalRead(reed_pin);
    current_open = !last_read;
    last_debounce = millis();
}

void reed_update()
{
    bool reading = digitalRead(reed_pin);
    if (reading != last_read)
    {
        last_debounce = millis();
        last_read = reading;
    }
    if ((millis() - last_debounce) > debounce_ms)
    {
        bool new_open = !reading;
        if (new_open != current_open)
        {
            current_open = new_open;
            if (change_cb)
                change_cb(current_open);
        }
    }
}

bool reed_is_open()
{
    return current_open;
}

void reed_set_callback(void (*cb)(bool))
{
    change_cb = cb;
}