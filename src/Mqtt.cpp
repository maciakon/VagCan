#include "Mqtt.h"

const String PubTopic = "remotecardiagz/pids/";
const char *SubTopic = "remotecardiagz/activemeasurements";

Mqtt::Mqtt(activatedPidsKeyValuePair* activePids)
{
    _activePids = activePids;
}

void Mqtt::onWifiConnect(const WiFiEventStationModeGotIP &event)
{
    this->subscribeToMqttEvents();
    this->connectToMqtt();
}

void Mqtt::onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
    Serial.println("Subscribing to MQTT events...");
    _mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
}

void Mqtt::subscribeToMqttEvents()
{
    Serial.println("Subscribing to MQTT events...");
    mqttClient.onConnect(std::bind(&Mqtt::onMqttConnect, this, std::placeholders::_1));
    mqttClient.onDisconnect(std::bind(&Mqtt::onMqttDisconnect, this, std::placeholders::_1));
    mqttClient.onSubscribe(std::bind(&Mqtt::onMqttSubscribe, this, std::placeholders::_1, std::placeholders::_2));
    mqttClient.onUnsubscribe(std::bind(&Mqtt::onMqttUnsubscribe, this, std::placeholders::_1));
    mqttClient.onMessage(std::bind(&Mqtt::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    mqttClient.onPublish(std::bind(&Mqtt::onMqttPublish, this, std::placeholders::_1));
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

void Mqtt::connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void Mqtt::publishMessage(String topic, byte value)
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

void Mqtt::onMqttConnect(bool sessionPresent)
{
    Serial.print("Connected to MQTT broker: ");
    Serial.print(MQTT_HOST);
    Serial.print(", port: ");
    Serial.println(MQTT_PORT);
    Serial.print("PubTopic: ");
    Serial.println(PubTopic);

    printSeparationLine();
    Serial.print("Session present: ");
    Serial.println(sessionPresent);

    uint16_t packetIdSub = mqttClient.subscribe(SubTopic, 2);
    Serial.print("Subscribing at QoS 2, packetId: ");
    Serial.println(packetIdSub);
    

    printSeparationLine();
}

void Mqtt::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    (void)reason;

    Serial.println("Disconnected from MQTT.");

    if (WiFi.isConnected())
    {
        _mqttReconnectTimer.once(2, std::bind(&Mqtt::connectToMqtt, this));
    }
}

void Mqtt::onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos)
{
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
}

void Mqtt::onMqttUnsubscribe(const uint16_t &packetId)
{
    Serial.println("Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void Mqtt::onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties,
                         const size_t &len, const size_t &index, const size_t &total)
{
    if(strcmp(topic, SubTopic) != 0)
    {
        return;
    }
    Serial.println("Message received.");
    Serial.print("  topic: ");
    Serial.println(topic);
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

        for(int i = 0; i < 10; i++)
        {
            if(_activePids[i].pidId == pidValue)
            {
                _activePids[i].isActive = isActive;
            }
        }
    }
}


void Mqtt::onMqttPublish(const uint16_t &packetId)
{
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void Mqtt::printSeparationLine()
{
    Serial.println("************************************************");
}