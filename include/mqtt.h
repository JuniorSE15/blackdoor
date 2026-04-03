#ifndef MQTT_H
#define MQTT_H

#include <PubSubClient.h>

const String TOPICPREFIX = "blackdoor/";

bool connectToWiFi(const char* ssid, const char* password);
void setupMQTT(const char* id, const char* broker, int port);
bool connectToMQTT();
bool publishState(const char* state);
void handleMQTTEvent(void* pvParameters);

#endif // MQTT_H