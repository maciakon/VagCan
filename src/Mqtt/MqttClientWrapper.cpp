#include "Mqtt\MqttClientWrapper.h"

MqttClientWrapper::MqttClientWrapper(activatedPidsKeyValuePair *activePids) : _measurementsMqttClient(activePids), _initialConfigMqttClient(activePids) { }

void MqttClientWrapper::onWifiConnect(const WiFiEventStationModeGotIP &event)
{
     this->subscribeToMqttEvents();
     this->connectToMqtt();
}

void MqttClientWrapper::onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
     Serial.println("Wifi disconnected in client base.");
     _mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
}

void MqttClientWrapper::publishMeasurementMessage(String topic, uint16_t value)
{
     _measurementsMqttClient.publishMessage(topic, value);
}

void MqttClientWrapper::connectToMqtt()
{
     Serial.println("Connecting to MQTT...");
     if (!mqttClient.connected())
     {
          mqttClient.connect();
     }
}

void MqttClientWrapper::subscribeToMqttEvents()
{
     Serial.println("Subscribing to MQTT events...");
     mqttClient.onConnect(std::bind(&MqttClientWrapper::onMqttConnect, this, std::placeholders::_1));                                                                                                                    // subscriptionToTopic
     mqttClient.onDisconnect(std::bind(&MqttClientWrapper::onMqttDisconnect, this, std::placeholders::_1));                                                                                                              // base reconnection
     mqttClient.onSubscribe(std::bind(&MqttClientWrapper::onMqttSubscribe, this, std::placeholders::_1, std::placeholders::_2));                                                                                         // InitialConfigOnly
     mqttClient.onUnsubscribe(std::bind(&MqttClientWrapper::onMqttUnsubscribe, this, std::placeholders::_1));                                                                                                            // base just logging
     mqttClient.onMessage(std::bind(&MqttClientWrapper::onMqttMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6)); // both clients
     mqttClient.onPublish(std::bind(&MqttClientWrapper::onMqttPublish, this, std::placeholders::_1));                                                                                                                    // base just log publishing
     mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

void MqttClientWrapper::onMqttConnect(bool sessionPresent)
{
     Serial.print("Connected to MQTT broker: ");
     Serial.print(MQTT_HOST);
     Serial.print(", port: ");
     Serial.println(MQTT_PORT);

     printSeparationLine();
     Serial.print("Session present: ");
     Serial.println(sessionPresent);

     _initialConfigPacketIdSub = mqttClient.subscribe(_initialConfigMqttClient.InitialConfigSubTopic, 2);
     uint16_t measurementsPacketIdSub = mqttClient.subscribe(_initialConfigMqttClient.MeasurementsChangedSubTopic, 2);
     Serial.print("Subscribing at QoS 2, packetId: ");
     Serial.println(_initialConfigPacketIdSub);

     Serial.print("Subscribing at QoS 2, packetId: ");
     Serial.println(measurementsPacketIdSub);

     printSeparationLine();
}

void MqttClientWrapper::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
     Serial.println("Disconnected from MQTT. Reason:");
     Serial.println((uint8_t)reason);

     if (WiFi.isConnected())
     {
          _mqttReconnectTimer.once(2, std::bind(&MqttClientWrapper::connectToMqtt, this));
     }
}

void MqttClientWrapper::onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos)
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

void MqttClientWrapper::onMqttUnsubscribe(const uint16_t &packetId)
{
     Serial.println("Unsubscribe acknowledged.");
     Serial.print("  packetId: ");
     Serial.println(packetId);
}

void MqttClientWrapper::onMqttPublish(const uint16_t &packetId)
{
     Serial.println("Publish acknowledged.");
     Serial.print("  packetId: ");
     Serial.println(packetId);
}

void MqttClientWrapper::printSeparationLine()
{
     Serial.println("************************************************");
}

void MqttClientWrapper::onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties, const size_t &len, const size_t &index, const size_t &total)
{
     if (strcmp(topic, _initialConfigMqttClient.InitialConfigSubTopic) == 0)
     {
          _initialConfigMqttClient.handleInitialConfigMessage(payload);
     }

     if (strcmp(topic, _initialConfigMqttClient.MeasurementsChangedSubTopic) == 0)
     {
          _initialConfigMqttClient.handleConfigChangedMessage(payload);
     }
}
