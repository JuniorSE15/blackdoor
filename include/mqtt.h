#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>

const String TOPICPREFIX = "blackdoor/";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

static String deviceId;
static String stateTopic;
static String actionTopic;

void mqttCallback(char* topic, byte* payload, unsigned int length);
bool connectToWiFi(const char* ssid, const char* password);
PubSubClient& setupMQTT(const char* id, const char* broker, int port);
bool connectToMQTT();
bool publishState(const char* state);

#endif // MQTT_H