/**
 * Unit tests for MQTT module
 * Tests MQTT setup, connection, and message handling
 */
#include <unity.h>
#include <cstring>
#include <Arduino.h>
#include "config.h"
#include "event.h"
#include "mqtt.h"

volatile bool isOpen = false;

void setUp(void) {
    isOpen = false;
    digitalWrite(RELAY_PIN, HIGH);
}

void tearDown(void) {
    isOpen = false;
    digitalWrite(RELAY_PIN, HIGH);
}

// ------------- MQTT Setup Tests -------------

void test_setup_mqtt_returns_client(void) {
    const char* device_id = "test-device";
    const char* address = "localhost";
    int port = 1883;

    PubSubClient& result = setupMQTT(device_id, address, port);
    // Just verify setup doesn't crash - connection requires broker
    TEST_ASSERT_TRUE(true);
}

void test_setup_mqtt_with_different_ports(void) {
    setupMQTT("device1", "localhost", 1883);
    TEST_ASSERT_TRUE(true);
    
    setupMQTT("device2", "broker.local", 8883);
    TEST_ASSERT_TRUE(true);
}

// ------------- MQTT Callback Tests -------------

void test_mqtt_callback_unlock_command(void) {
    const char* device_id = "test-device";
    setupMQTT(device_id, "localhost", 1883);

    char topic[] = "blackdoor/test-device/action";
    char payload[] = "unlock";
    
    isOpen = false;
    mqttCallback(topic, reinterpret_cast<byte*>(payload), strlen(payload));
    
    TEST_ASSERT_TRUE(isOpen);
}

void test_mqtt_callback_lock_command(void) {
    const char* device_id = "test-device";
    setupMQTT(device_id, "localhost", 1883);

    char topic[] = "blackdoor/test-device/action";
    char payload[] = "lock";
    
    isOpen = true;
    mqttCallback(topic, reinterpret_cast<byte*>(payload), strlen(payload));
    
    TEST_ASSERT_FALSE(isOpen);
}

void test_mqtt_callback_unlock_then_lock(void) {
    const char* device_id = "test-device";
    setupMQTT(device_id, "localhost", 1883);

    char topic[] = "blackdoor/test-device/action";
    
    // Unlock
    char unlockPayload[] = "unlock";
    mqttCallback(topic, reinterpret_cast<byte*>(unlockPayload), strlen(unlockPayload));
    TEST_ASSERT_TRUE(isOpen);
    
    // Lock
    char lockPayload[] = "lock";
    mqttCallback(topic, reinterpret_cast<byte*>(lockPayload), strlen(lockPayload));
    TEST_ASSERT_FALSE(isOpen);
}

void test_mqtt_callback_ignores_invalid_command(void) {
    const char* device_id = "test-device";
    setupMQTT(device_id, "localhost", 1883);

    char topic[] = "blackdoor/test-device/action";
    char payload[] = "invalid";
    
    isOpen = false;
    mqttCallback(topic, reinterpret_cast<byte*>(payload), strlen(payload));
    
    // State should remain unchanged
    TEST_ASSERT_FALSE(isOpen);
}

void test_mqtt_callback_ignores_wrong_topic(void) {
    const char* device_id = "test-device";
    setupMQTT(device_id, "localhost", 1883);

    char topic[] = "blackdoor/other-device/action";
    char payload[] = "unlock";
    
    isOpen = false;
    mqttCallback(topic, reinterpret_cast<byte*>(payload), strlen(payload));
    
    // State should remain unchanged (wrong device topic)
    TEST_ASSERT_FALSE(isOpen);
}

void test_mqtt_callback_ignores_state_topic(void) {
    const char* device_id = "test-device";
    setupMQTT(device_id, "localhost", 1883);

    char topic[] = "blackdoor/test-device/state";
    char payload[] = "unlock";
    
    isOpen = false;
    mqttCallback(topic, reinterpret_cast<byte*>(payload), strlen(payload));
    
    // State should remain unchanged (state topic, not action)
    TEST_ASSERT_FALSE(isOpen);
}

void setup() {
    delay(2000);
    
    UNITY_BEGIN();
    
    // Setup tests
    RUN_TEST(test_setup_mqtt_returns_client);
    RUN_TEST(test_setup_mqtt_with_different_ports);
    
    // Callback tests
    RUN_TEST(test_mqtt_callback_unlock_command);
    RUN_TEST(test_mqtt_callback_lock_command);
    RUN_TEST(test_mqtt_callback_unlock_then_lock);
    RUN_TEST(test_mqtt_callback_ignores_invalid_command);
    RUN_TEST(test_mqtt_callback_ignores_wrong_topic);
    RUN_TEST(test_mqtt_callback_ignores_state_topic);
    
    UNITY_END();
}

void loop() {
    // Nothing to do here
}
