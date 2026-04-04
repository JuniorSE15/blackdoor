#include <unity.h>

#include "mqtt.h"

PubSubClient mqttClient;

void setUp() {}
void tearDown() {}

int main( int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(testSetupMQTT);
    RUN_TEST(testConnectToMQTT);
    RUN_TEST(testPublishState);

    UNITY_END();
}

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