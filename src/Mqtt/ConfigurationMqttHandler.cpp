#include "Mqtt\ConfigurationMqttHandler.h"

ConfigurationMqttHandler::ConfigurationMqttHandler(activatedPidsKeyValuePair *activePids)
{
    _activePids = activePids;
}

void ConfigurationMqttHandler::publishInitialConfigurationMessage()
{
    String publishTopic = InitialConfigPubTopic;
    Serial.println("Publishing initial config message to topic:");
    Serial.println(publishTopic.c_str());
    uint16_t packetIdPub1 = mqttClient.publish(publishTopic.c_str(), 0, true);
    Serial.print("Publishing initial config message at QoS 1, packetId: ");
    Serial.println(packetIdPub1);
}

void ConfigurationMqttHandler::handleInitialConfigMessage(char *payload)
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

void ConfigurationMqttHandler::handleConfigChangedMessage(char *payload)
{
    Serial.println("Measuremets client handling message:");
    Serial.println("    payload:    ");
    Serial.println(payload);
    StaticJsonDocument<61> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
        Serial.print("Serialization ERROR ");
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
    }
    else
    {
        byte pidValue = doc["Value"];
        Serial.println("item value");
        Serial.println(pidValue, DEC);

        bool isActive = doc["IsActive"];
        Serial.println("IsActive");
        Serial.println(isActive);

        for (int i = 0; i < 10; i++)
        {
            if (_activePids[i].pidId == pidValue)
            {
                _activePids[i].isActive = isActive;
            }
        }
    }
}