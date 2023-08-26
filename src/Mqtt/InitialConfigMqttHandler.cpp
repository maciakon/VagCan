#include "Mqtt\InitialConfigMqttHandler.h"

InitialConfigMqttHandler::InitialConfigMqttHandler(activatedPidsKeyValuePair *activePids)
{
    _activePids = activePids;
}

void InitialConfigMqttHandler::publishInitialConfigurationMessage()
{
    String publishTopic = InitialConfigPubTopic;
    Serial.println("Publishing initial config message to topic:");
    Serial.println(publishTopic.c_str());
    uint16_t packetIdPub1 = mqttClient.publish(publishTopic.c_str(), 0, true);
    Serial.print("Publishing initial config message at QoS 1, packetId: ");
    Serial.println(packetIdPub1);
}

void InitialConfigMqttHandler::handleMessage(char *payload)
{
    Serial.println("InitialConfig client message received.");
    Serial.print("  topic: ");
    Serial.println("    payload:    ");
    Serial.println(payload);
    StaticJsonDocument<768> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.print("Serialization ERROR ");
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
    }
    else
    {
        int i = 0;
        for (JsonObject item : doc.as<JsonArray>())
        {
            int pidValue = item["Value"];
            bool isActive = item["IsActive"];
            _activePids[i].pidId = pidValue;
            _activePids[i].isActive = isActive;
            i++;
        }
    }
}