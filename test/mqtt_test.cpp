#include <unity.h>
#include <cstring>

#include "mqtt.h"

extern volatile bool isOpen; // door state defined in door_test.cpp

void testSetupMQTT() {
    const char* device_id = "test-device";
    const char* address = "localhost";
    int port = 1883;

    PubSubClient& result = setupMQTT(device_id, address, port);
    TEST_ASSERT_TRUE(result.connected());
}

void testConnectToMQTT() {
    // This test assumes that the MQTT broker is running and accessible
    const char* device_id = "test-device";
    const char* address = "localhost";
    int port = 1883;

    setupMQTT(device_id, address, port);
    bool result = connectToMQTT();
    TEST_ASSERT_TRUE(result);
}

void testPublishState() {
    // This test assumes that the MQTT broker is running and accessible
    const char* device_id = "test-device";
    const char* address = "localhost";
    int port = 1883;

    setupMQTT(device_id, address, port);
    connectToMQTT();
    
    bool result = publishState("unlocked");
    TEST_ASSERT_TRUE(result);
}

void mqttCallBackTest() {
    const char* device_id = "test-device";
    const char* address = "localhost";
    int port = 1883;

    setupMQTT(device_id, address, port);

    char topic[] = "blackdoor/test-device/action";

    char unlockPayload[] = "unlock";
    mqttCallback(topic, reinterpret_cast<byte*>(unlockPayload), strlen(unlockPayload));
    TEST_ASSERT_TRUE(isOpen);

    char lockPayload[] = "lock";
    mqttCallback(topic, reinterpret_cast<byte*>(lockPayload), strlen(lockPayload));
    TEST_ASSERT_FALSE(isOpen);
}
