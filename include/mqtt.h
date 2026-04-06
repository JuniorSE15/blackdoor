#ifndef MQTT_H
#define MQTT_H

#include <WiFi.h>
#include <PubSubClient.h>

extern WiFiClient wifiClient;
extern PubSubClient mqttClient;

void mqttCallback(char* topic, byte* payload, unsigned int length);
bool connectToWiFi(const char* ssid, const char* password);
PubSubClient& setupMQTT(const char* id, const char* broker, int port);
bool connectToMQTT();
bool publishState(const char* state);

#endif // MQTT_H