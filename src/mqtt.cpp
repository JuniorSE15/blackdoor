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
static String cardsTopic;

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
            if (doorPhysicallyOpen)
            {
                // Solenoid bolt cannot engage while the door is open — the bolt
                // would slam into the strike plate and jam. Reject the command.
                Serial.println("MQTT: lock command IGNORED — door is physically open.");
                publishState("open"); // remind the app of the actual state
            }
            else
            {
                lockDoor();
                Serial.println("MQTT: Door locked");
                publishState("locked");
            }
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
            else if (type == "enroll_mode")
            {
                // {"type":"enroll_mode","enabled":"true"}  → enter ADMIN_MODE remotely
                // {"type":"enroll_mode","enabled":"false"} → exit immediately
                String enabled = extractJsonString(message, "enabled");
                if (enabled == "true")
                {
                    enterAdminMode(true); // remote: auto-exit after first card scan
                    Serial.println("MQTT: Enrollment mode started (auto-exit after one scan)");
                }
                else
                {
                    exitAdminMode();
                    Serial.println("MQTT: Enrollment mode stopped");
                }
            }
            else if (type == "add_card")
            {
                // {"type":"add_card","uid":"a1b2c3d4"}
                // Option A: iPhone scanned the card and sends UID directly.
                String uid = extractJsonString(message, "uid");
                uid.toLowerCase(); // normalise to match PN532 output
                if (uid.length() > 0)
                {
                    if (addCard(uid))
                    {
                        Serial.printf("MQTT: Card added: %s\n", uid.c_str());
                        publishCards(); // sync updated list back to backend
                    }
                    else
                    {
                        Serial.printf("MQTT: add_card failed — duplicate or database full (%s)\n", uid.c_str());
                    }
                }
                else
                {
                    Serial.println("MQTT: add_card — missing uid field");
                }
            }
            else if (type == "remove_card")
            {
                // {"type":"remove_card","uid":"a1b2c3d4"}
                String uid = extractJsonString(message, "uid");
                uid.toLowerCase();
                if (uid.length() > 0)
                {
                    if (revokeCard(uid))
                    {
                        Serial.printf("MQTT: Card revoked: %s\n", uid.c_str());
                        publishCards(); // sync updated list back to backend
                    }
                    else
                    {
                        Serial.printf("MQTT: remove_card — card not found: %s\n", uid.c_str());
                    }
                }
            }
            else if (type == "list_cards")
            {
                // {"type":"list_cards"} — on-demand request to publish the card list
                Serial.println("MQTT: list_cards requested");
                publishCards();
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
    deviceId = String(id);
    stateTopic  = String(TOPICPREFIX) + deviceId + "/state";
    actionTopic = String(TOPICPREFIX) + deviceId + "/action";
    cardsTopic  = String(TOPICPREFIX) + deviceId + "/cards";

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

        // Publish full card list so the backend DB stays in sync
        publishCards();

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

// Publishes the full authorized card list as a retained JSON message.
// Topic: blackdoor/{deviceId}/cards
// Payload: {"cards":["uid1","uid2",...]}
// Called on connect, after add/remove via MQTT, and on list_cards command.
bool publishCards()
{
    if (!mqttClient.connected())
    {
        return false;
    }

    String payload = "{\"cards\":[";
    int count = getCardCount();
    for (int i = 0; i < count; i++)
    {
        if (i > 0)
            payload += ",";
        payload += "\"" + getCardAt(i) + "\"";
    }
    payload += "]}";

    Serial.printf("[MQTT] Publishing card list (%d card(s)): %s\n", count, payload.c_str());
    return mqttClient.publish(cardsTopic.c_str(), payload.c_str(), true); // retained
}