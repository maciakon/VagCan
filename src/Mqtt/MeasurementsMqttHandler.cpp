#include "Mqtt\MeasurementsMqttHandler.h"

MeasurementsMqttHandler::MeasurementsMqttHandler(activatedPidsKeyValuePair *activePids) {
    _activePids = activePids;
}

void MeasurementsMqttHandler::publishMessage(String topic, byte value)
{
    String publishTopic = PubTopic + topic;
    Serial.println("pubishing to topic:");
    Serial.println(publishTopic.c_str());
    char sensorValue[4];
    sprintf_P(sensorValue, "%d", value);
    uint16_t packetIdPub1 = mqttClient.publish(publishTopic.c_str(), 0, true, sensorValue);
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPub1);
}

void MeasurementsMqttHandler::handleMessage(char *payload)
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
