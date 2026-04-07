#include "mqtt.h"
#include "access_control.h"
#include "esp_task_wdt.h"
#include <WiFi.h>

const String TOPICPREFIX = "blackdoor/";

// ─── Minimal JSON string extractor ───────────────────────────────────────────
// Finds `"key":"value"` in a flat JSON object and returns the value.
// Sufficient for the set_pin payload; no library needed.
static String extractJsonString(const String &json, const String &key)
{
    String searchKey = "\"" + key + "\":\"";
    int idx = json.indexOf(searchKey);
    if (idx == -1)
        return "";
    int start = idx + searchKey.length();
    int end = json.indexOf('"', start);
    if (end == -1)
        return "";
    return json.substring(start, end);
}

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

static String deviceId;
static String stateTopic;
static String actionTopic;

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    // debugging purpose
    Serial.print("MQTT received on ");
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(message);

    if (String(topic) == actionTopic)
    {
        if (message == "unlock")
        {
            grantAccess(AccessSource::UWB); // reuse UWB source for remote unlock
            Serial.println("MQTT: Door unlocked");
            publishState("unlocked");
        }
        else if (message == "lock")
        {
            lockDoor();
            Serial.println("MQTT: Door locked");
            publishState("locked");
        }
        else if (message.startsWith("{"))
        {
            String type = extractJsonString(message, "type");
            if (type == "set_pin")
            {
                String newPin = extractJsonString(message, "pin");
                if (setPin(newPin))
                {
                    Serial.println("MQTT: PIN updated successfully");
                }
                else
                {
                    Serial.println("MQTT: set_pin failed (PIN too short?)");
                }
            }
        }
    }
}

bool connectToWiFi(const char *ssid, const char *password)
{
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    // debugging purpose
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi connected");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        return true;
    }

    Serial.println("\nWiFi connection failed");
    return false;
}

PubSubClient &setupMQTT(const char *id, const char *address, int port)
{
    // deviceId = String(id);
    stateTopic = String(TOPICPREFIX) + "+" + "/state";
    actionTopic = String(TOPICPREFIX) + "+" + "/action";

    mqttClient.setServer(address, port);
    mqttClient.setCallback(mqttCallback);
    return mqttClient;
}

bool connectToMQTT(const char *user, const char *password)
{
    if (mqttClient.connected())
    {
        return true;
    }

    Serial.print("Connecting to MQTT broker...");

    String clientId = "blackdoor-" + deviceId;

    if (mqttClient.connect(clientId.c_str(), user, password))
    {
        Serial.println("connected");

        // Subscribe to action topic
        mqttClient.subscribe(actionTopic.c_str());
        Serial.print("Subscribed to: ");
        Serial.println(actionTopic);

        // Publish initial state
        publishState(isOpen ? "unlocked" : "locked");

        return true;
    }

    Serial.print("failed, rc=");
    Serial.println(mqttClient.state());
    return false;
}

bool publishState(const char *state)
{
    if (!mqttClient.connected())
    {
        return false;
    }
    return mqttClient.publish(stateTopic.c_str(), state, true); // retained
}