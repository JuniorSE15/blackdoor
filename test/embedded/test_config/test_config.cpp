/**
 * Embedded tests for config module - requires hardware connected
 * Tests hardware initialization with actual GPIO reads
 */
#include <unity.h>
#include <Arduino.h>
#include "config.h"

volatile bool isOpen = false;

void setUp(void) {
    isOpen = false;
}

void tearDown(void) {
    // Clean up after each test
}

void test_wakeup_hardware_sets_relay_high(void) {
    volatile bool testIsOpen = true;
    wakeUpHardware(&testIsOpen);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
}

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    RUN_TEST(test_wakeup_hardware_sets_relay_high);
    
    UNITY_END();
}

void loop() {
    // Nothing to do here
}
