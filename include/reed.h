#ifndef REED_H
#define REED_H

#include <stdint.h>

void reed_init(uint8_t pin);
void reed_update();
bool reed_is_open();
void reed_set_callback(void (*cb)(bool open));

#endif
