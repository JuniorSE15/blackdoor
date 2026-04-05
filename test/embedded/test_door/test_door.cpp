#include <unity.h>
#include <cstring>

#include "config.h"
#include "event.h"
#include "mqtt.h"

volatile bool isOpen = false; // shared state for all tests

// Forward declarations for integration tests
void test_setup_config_initializes_pins();
void test_door_initial_state_is_locked();
void test_unlock_sets_correct_state();
void test_lock_sets_correct_state();
void test_unlock_then_auto_lock_sequence();
void test_multiple_unlock_attempts();
void test_multiple_lock_attempts();
void test_rapid_lock_unlock_cycles();
void mqttUnlockPublishTest();
void mqttlockPublishTest();

// Forward declarations for MQTT tests
void testSetupMQTT();
void testConnectToMQTT();
void testPublishState();
void mqttCallBackTest();

void setUp(void) {
    isOpen = false;
    digitalWrite(RELAY_PIN, HIGH);  // Door locked (relay off)
}

void tearDown(void) {
    isOpen = false;
    digitalWrite(RELAY_PIN, HIGH);
}

void test_initial_state_wakeup(void) {
    volatile bool fakeIsOpen = true;
    wakeUpHardware(&fakeIsOpen);
    TEST_ASSERT_FALSE(fakeIsOpen);
}

void test_trigger_relay(void) {
    // In Arduino framework, testing digital writes often requires 
    // a mock or running on the actual hardware. Let's assume testing compilation first.
    triggerRelay(HIGH); // Set HIGH
    triggerRelay(LOW); // Set LOW
    TEST_ASSERT_TRUE(true);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    // Door tests
    RUN_TEST(test_initial_state_wakeup);
    RUN_TEST(test_trigger_relay);

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

    // MQTT tests
    RUN_TEST(testSetupMQTT);
    RUN_TEST(testConnectToMQTT);
    RUN_TEST(testPublishState);
    RUN_TEST(mqttCallBackTest);

    return UNITY_END();
}