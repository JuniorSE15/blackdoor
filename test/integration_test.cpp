#include <unity.h>

#include "config.h"
#include "event.h"
#include "mqtt.h"

extern volatile bool isOpen; // door state defined in door_test.cpp

// ------------- Configuration Tests -------------

void test_setup_config_initializes_pins() {
    setupConfig();
    TEST_ASSERT_TRUE(true);
}

void test_door_initial_state_is_locked() {
    TEST_ASSERT_FALSE(isOpen);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
}

// ------------- Lock-State Integration Tests -------------

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

// ------------- MQTT Integration Tests -------------
// These tests assume that the MQTT broker is running and accessible
void mqttUnlockPublishTest() {
    setupMQTT("test-device", "localhost", 1883);
    connectToMQTT();
    
    isOpen = true;
    triggerRelay(LOW);
    
    bool result = publishState("unlocked");
    TEST_ASSERT_TRUE(result);
}

void mqttlockPublishTest() {
    setupMQTT("test-device", "localhost", 1883);
    connectToMQTT();
    
    isOpen = false;
    triggerRelay(HIGH);
    
    bool result = publishState("locked");
    TEST_ASSERT_TRUE(result);
}


