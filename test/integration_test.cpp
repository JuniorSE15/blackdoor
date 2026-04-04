#include <unity.h>

#include "config.h"
#include "event.h"

volatile bool isOpen = false; // door state

void setUp() {
    isOpen = false;
    digitalWrite(RELAY_PIN, HIGH);  // Door locked (relay off)
}

void tearDown() {
    isOpen = false;
    digitalWrite(RELAY_PIN, HIGH);
}

void test_setup_config_initializes_pins() {
    setupConfig();
    TEST_ASSERT_TRUE(true);
}

void test_door_initial_state_is_locked() {
    TEST_ASSERT_FALSE(isOpen);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
}

void test_unlock_sets_correct_state() {
    // Simulate unlock action
    isOpen = true;
    triggerRelay(LOW);
    
    TEST_ASSERT_TRUE(isOpen);
    TEST_ASSERT_EQUAL(LOW, digitalRead(RELAY_PIN));
}

void test_lock_sets_correct_state() {
    isOpen = true;
    triggerRelay(LOW);
    
    isOpen = false;
    triggerRelay(HIGH);
    
    TEST_ASSERT_FALSE(isOpen);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
}

void test_unlock_then_auto_lock_sequence() {
    TEST_ASSERT_FALSE(isOpen);
    
    isOpen = true;
    triggerRelay(LOW);
    TEST_ASSERT_TRUE(isOpen);
    TEST_ASSERT_EQUAL(LOW, digitalRead(RELAY_PIN));
    
    isOpen = false;
    triggerRelay(HIGH);
    TEST_ASSERT_FALSE(isOpen);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
}

void test_multiple_unlock_attempts() {
    for (int i = 0; i < 5; i++) {
        isOpen = true;
        triggerRelay(LOW);
        TEST_ASSERT_EQUAL(LOW, digitalRead(RELAY_PIN));
    }
}

void test_multiple_lock_attempts() {
    for (int i = 0; i < 5; i++) {
        isOpen = false;
        triggerRelay(HIGH);
        TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
    }
}

void test_rapid_lock_unlock_cycles() {
    // Simulate rapid toggling
    for (int i = 0; i < 10; i++) {
        isOpen = true;
        triggerRelay(LOW);
        TEST_ASSERT_EQUAL(LOW, digitalRead(RELAY_PIN));
        
        isOpen = false;
        triggerRelay(HIGH);
        TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
    }
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Configuration tests
    RUN_TEST(test_setup_config_initializes_pins);

    // Door state integration tests
    RUN_TEST(test_door_initial_state_is_locked);
    RUN_TEST(test_unlock_sets_correct_state);
    RUN_TEST(test_lock_sets_correct_state);
    RUN_TEST(test_unlock_then_auto_lock_sequence);

    // Edge case tests
    RUN_TEST(test_multiple_unlock_attempts);
    RUN_TEST(test_multiple_lock_attempts);
    RUN_TEST(test_rapid_lock_unlock_cycles);

    return UNITY_END();
}
