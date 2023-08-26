#include "Mqtt\MqttClientBase.h"

MqttClientBase::MqttClientBase(activatedPidsKeyValuePair *activePids) : _measurementsMqttClient(activePids), _initialConfigMqttClient(activePids)
{
     _activePids = activePids;
}

void MqttClientBase::onWifiConnect(const WiFiEventStationModeGotIP &event)
{
     this->subscribeToMqttEvents();
     this->connectToMqtt();
}

void MqttClientBase::onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
     Serial.println("Wifi disconnected in client base.");
     _mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
}

void MqttClientBase::publishMeasurementMessage(String topic, byte value)
{
     _measurementsMqttClient.publishMessage(topic, value);
}
void MqttClientBase::connectToMqtt()
{
     Serial.println("Connecting to MQTT...");
     if (!mqttClient.connected())
     {
          mqttClient.connect();
     }
}

void MqttClientBase::subscribeToMqttEvents()
{
     Serial.println("Subscribing to MQTT events...");
     mqttClient.onConnect(std::bind(&MqttClientBase::onMqttConnect, this, std::placeholders::_1));                                                                                                                    // subscriptionToTopic
     mqttClient.onDisconnect(std::bind(&MqttClientBase::onMqttDisconnect, this, std::placeholders::_1));                                                                                                              // base reconnection
     mqttClient.onSubscribe(std::bind(&MqttClientBase::onMqttSubscribe, this, std::placeholders::_1, std::placeholders::_2));                                                                                         // InitialConfigOnly
     mqttClient.onUnsubscribe(std::bind(&MqttClientBase::onMqttUnsubscribe, this, std::placeholders::_1));                                                                                                            // base just logging
     mqttClient.onMessage(std::bind(&MqttClientBase::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)); // both clients
     mqttClient.onPublish(std::bind(&MqttClientBase::onMqttPublish, this, std::placeholders::_1));                                                                                                                    // base just log publishing
     mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

void MqttClientBase::onMqttConnect(bool sessionPresent)
{
     Serial.print("Connected to MQTT broker: ");
     Serial.print(MQTT_HOST);
     Serial.print(", port: ");
     Serial.println(MQTT_PORT);

     printSeparationLine();
     Serial.print("Session present: ");
     Serial.println(sessionPresent);

     _initialConfigPacketIdSub = mqttClient.subscribe(_initialConfigMqttClient.InitialConfigSubTopic, 2);
     uint16_t measurementsPacketIdSub = mqttClient.subscribe(_measurementsMqttClient.SubTopic, 2);
     Serial.print("Subscribing at QoS 2, packetId: ");
     Serial.println(_initialConfigPacketIdSub);

     Serial.print("Subscribing at QoS 2, packetId: ");
     Serial.println(measurementsPacketIdSub);

     printSeparationLine();
}

void MqttClientBase::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
     (void)reason;

     Serial.println("Disconnected from MQTT.");

     if (WiFi.isConnected())
     {
          _mqttReconnectTimer.once(2, std::bind(&MqttClientBase::connectToMqtt, this));
     }
}

void MqttClientBase::onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos)
{
     Serial.println("Subscribe acknowledged.");
     Serial.print("  packetId: ");
     Serial.println(packetId);
     Serial.print("  qos: ");
     Serial.println(qos);
     if (packetId == _initialConfigPacketIdSub)
     {
          _initialConfigMqttClient.publishInitialConfigurationMessage();
     }
}

void MqttClientBase::onMqttUnsubscribe(const uint16_t &packetId)
{
     Serial.println("Unsubscribe acknowledged.");
     Serial.print("  packetId: ");
     Serial.println(packetId);
}

void MqttClientBase::onMqttPublish(const uint16_t &packetId)
{
     Serial.println("Publish acknowledged.");
     Serial.print("  packetId: ");
     Serial.println(packetId);
}

void MqttClientBase::printSeparationLine()
{
     Serial.println("************************************************");
}

void MqttClientBase::onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties, const size_t &len, const size_t &index, const size_t &total)
{
     if (strcmp(topic, _initialConfigMqttClient.InitialConfigSubTopic) == 0)
     {
          _initialConfigMqttClient.handleMessage(payload);
     }

     if (strcmp(topic, _measurementsMqttClient.SubTopic) == 0)
     {
          _measurementsMqttClient.handleMessage(payload);
     }
}
