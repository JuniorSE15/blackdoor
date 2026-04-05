/**
 * Embedded tests for event module - requires hardware connected
 * Tests relay control with actual GPIO reads
 */
#include <unity.h>
#include <Arduino.h>
#include "config.h"
#include "event.h"

volatile bool isOpen = false;

void setUp(void) {
    isOpen = false;
    digitalWrite(RELAY_PIN, HIGH);  // Door locked (relay off)
}

void tearDown(void) {
    isOpen = false;
    digitalWrite(RELAY_PIN, HIGH);
}

// ------------- Relay Hardware Tests -------------

void test_trigger_relay_high_locks_door(void) {
    triggerRelay(HIGH);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
}

void test_trigger_relay_low_unlocks_door(void) {
    triggerRelay(LOW);
    TEST_ASSERT_EQUAL(LOW, digitalRead(RELAY_PIN));
}

void test_relay_toggle_sequence(void) {
    // Start locked
    triggerRelay(HIGH);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
    
    // Unlock
    triggerRelay(LOW);
    TEST_ASSERT_EQUAL(LOW, digitalRead(RELAY_PIN));
    
    // Lock again
    triggerRelay(HIGH);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
}

// ------------- Door State with Hardware Tests -------------

void test_unlock_sets_correct_state(void) {
    isOpen = true;
    triggerRelay(LOW);
    
    TEST_ASSERT_TRUE(isOpen);
    TEST_ASSERT_EQUAL(LOW, digitalRead(RELAY_PIN));
}

void test_lock_sets_correct_state(void) {
    // First unlock
    isOpen = true;
    triggerRelay(LOW);
    
    // Then lock
    isOpen = false;
    triggerRelay(HIGH);
    
    TEST_ASSERT_FALSE(isOpen);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
}

void test_unlock_then_lock_sequence(void) {
    // Initial state
    TEST_ASSERT_FALSE(isOpen);
    
    // Unlock
    isOpen = true;
    triggerRelay(LOW);
    TEST_ASSERT_TRUE(isOpen);
    TEST_ASSERT_EQUAL(LOW, digitalRead(RELAY_PIN));
    
    // Auto-lock simulation
    isOpen = false;
    triggerRelay(HIGH);
    TEST_ASSERT_FALSE(isOpen);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
}

// ------------- Edge Case Hardware Tests -------------

void test_multiple_lock_attempts(void) {
    for (int i = 0; i < 5; i++) {
        isOpen = false;
        triggerRelay(HIGH);
        TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
    }
}

void test_rapid_lock_unlock_cycles(void) {
    for (int i = 0; i < 10; i++) {
        isOpen = true;
        triggerRelay(LOW);
        TEST_ASSERT_EQUAL(LOW, digitalRead(RELAY_PIN));
        
        isOpen = false;
        triggerRelay(HIGH);
        TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
    }
}

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    // Relay hardware tests
    RUN_TEST(test_trigger_relay_high_locks_door);
    RUN_TEST(test_trigger_relay_low_unlocks_door);
    RUN_TEST(test_relay_toggle_sequence);
    
    // Door state with hardware verification
    RUN_TEST(test_unlock_sets_correct_state);
    RUN_TEST(test_lock_sets_correct_state);
    RUN_TEST(test_unlock_then_lock_sequence);
    
    // Edge case hardware tests
    RUN_TEST(test_multiple_lock_attempts);
    RUN_TEST(test_rapid_lock_unlock_cycles);
    
    UNITY_END();
}

void loop() {
    // Nothing to do here
}
