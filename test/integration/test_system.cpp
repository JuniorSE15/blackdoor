/**
 * Integration tests for full system
 * Tests MQTT + door state + hardware working together
 * Requires MQTT broker and ESP32 hardware
 */
#include <unity.h>
#include <Arduino.h>
#include <cstring>
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

// ------------- MQTT + Door Integration Tests -------------

void test_mqtt_unlock_updates_hardware(void) {
    setupMQTT("test-device", "localhost", 1883);
    
    // Simulate MQTT unlock command
    char topic[] = "blackdoor/test-device/action";
    char payload[] = "unlock";
    mqttCallback(topic, reinterpret_cast<byte*>(payload), strlen(payload));
    
    TEST_ASSERT_TRUE(isOpen);
}

void test_mqtt_lock_updates_hardware(void) {
    setupMQTT("test-device", "localhost", 1883);
    
    isOpen = true;
    
    char topic[] = "blackdoor/test-device/action";
    char payload[] = "lock";
    mqttCallback(topic, reinterpret_cast<byte*>(payload), strlen(payload));
    
    TEST_ASSERT_FALSE(isOpen);
}

void test_full_unlock_lock_cycle(void) {
    setupMQTT("test-device", "localhost", 1883);
    wakeUpHardware(&isOpen);
    
    TEST_ASSERT_FALSE(isOpen);
    TEST_ASSERT_EQUAL(HIGH, digitalRead(RELAY_PIN));
    
    // Unlock via MQTT
    char topic[] = "blackdoor/test-device/action";
    char unlockPayload[] = "unlock";
    mqttCallback(topic, reinterpret_cast<byte*>(unlockPayload), strlen(unlockPayload));
    
    TEST_ASSERT_TRUE(isOpen);
    
    // Lock via MQTT
    char lockPayload[] = "lock";
    mqttCallback(topic, reinterpret_cast<byte*>(lockPayload), strlen(lockPayload));
    
    TEST_ASSERT_FALSE(isOpen);
}

void setup() {
    delay(2000);
    
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
    
    UNITY_BEGIN();
    
    RUN_TEST(test_mqtt_unlock_updates_hardware);
    RUN_TEST(test_mqtt_lock_updates_hardware);
    RUN_TEST(test_full_unlock_lock_cycle);
    
    UNITY_END();
}

void loop() {
    // Nothing to do here
}
