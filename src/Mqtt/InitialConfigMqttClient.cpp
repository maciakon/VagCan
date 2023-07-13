#include "Mqtt\InitialConfigMqttClient.h"

const char *InitialConfigSubTopic = "remotecardiagz/initialconfiguration";

InitialConfigMqttClient::InitialConfigMqttClient(activatedPidsKeyValuePair* activePids)
{
    _activePids = activePids;
}

void InitialConfigMqttClient::onWifiConnect(const WiFiEventStationModeGotIP &event)
{
    this->subscribeToMqttEvents();
    this->connectToMqtt();
}

void InitialConfigMqttClient::onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
    Serial.println("Subscribing to MQTT events...");
    _mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
}

void InitialConfigMqttClient::subscribeToMqttEvents()
{
    Serial.println("Subscribing to MQTT events...");
    mqttClient.onConnect(std::bind(&InitialConfigMqttClient::onMqttConnect, this, std::placeholders::_1));
    mqttClient.onDisconnect(std::bind(&InitialConfigMqttClient::onMqttDisconnect, this, std::placeholders::_1));
    mqttClient.onSubscribe(std::bind(&InitialConfigMqttClient::onMqttSubscribe, this, std::placeholders::_1, std::placeholders::_2));
    mqttClient.onUnsubscribe(std::bind(&InitialConfigMqttClient::onMqttUnsubscribe, this, std::placeholders::_1));
    mqttClient.onMessage(std::bind(&InitialConfigMqttClient::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
    mqttClient.onPublish(std::bind(&InitialConfigMqttClient::onMqttPublish, this, std::placeholders::_1));
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

void InitialConfigMqttClient::connectToMqtt()
{
    Serial.println("Connecting to MQTT...");
    mqttClient.connect();
}

void InitialConfigMqttClient::publishInitialConfigurationMessage()
{
    String publishTopic = InitialConfigPubTopic;
    Serial.println("Publishing initial config message to topic:");
    Serial.println(publishTopic.c_str());
    uint16_t packetIdPub1 = mqttClient.publish(publishTopic.c_str(), 0, true);
    Serial.print("Publishing initial config message at QoS 1, packetId: ");
    Serial.println(packetIdPub1);
}

void InitialConfigMqttClient::onMqttConnect(bool sessionPresent)
{
    Serial.print("Connected to MQTT broker: ");
    Serial.print(MQTT_HOST);
    Serial.print(", port: ");
    Serial.println(MQTT_PORT);
    Serial.print("PubTopic: ");
    printSeparationLine();
    Serial.print("Session present: ");
    Serial.println(sessionPresent);

    uint16_t initialConfigPacketIdSub = mqttClient.subscribe(InitialConfigSubTopic, 2);
    Serial.print("Subscribing initial config at QoS 2, packetId: ");
    Serial.println(initialConfigPacketIdSub);
    

    printSeparationLine();
}

void InitialConfigMqttClient::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    (void)reason;

    Serial.println("Disconnected from MQTT.");

    if (WiFi.isConnected())
    {
        _mqttReconnectTimer.once(2, std::bind(&InitialConfigMqttClient::connectToMqtt, this));
    }
}

void InitialConfigMqttClient::onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos)
{
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
    publishInitialConfigurationMessage();
}

void InitialConfigMqttClient::onMqttUnsubscribe(const uint16_t &packetId)
{
    Serial.println("Unsubscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void InitialConfigMqttClient::onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties,
                         const size_t &len, const size_t &index, const size_t &total)
{
    if(strcmp(topic, InitialConfigSubTopic) != 0)
    {
        return;
    }
    Serial.println("InitialConfig client message received.");
    Serial.print("  topic: ");
    Serial.println(topic);
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

void InitialConfigMqttClient::onMqttPublish(const uint16_t &packetId)
{
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

void InitialConfigMqttClient::printSeparationLine()
{
    Serial.println("************************************************");
}