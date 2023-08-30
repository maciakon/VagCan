#ifndef MQTTCLIENTBASE_H
#define MQTTCLIENTBASE_H
#include <AsyncMqttClient_Generic.hpp>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "Can\pids.h"
#include "ConfigurationMqttHandler.h"
#include "MeasurementsMqttMessagePublisher.h"

#define MQTT_HOST "mqtt.remotecardiagz.pl" // 18.185.185.121
#define MQTT_PORT 1883

extern AsyncMqttClient mqttClient;

class MqttClientWrapper
{
public:
    MqttClientWrapper(activatedPidsKeyValuePair *activePids);
    void onWifiConnect(const WiFiEventStationModeGotIP &event);
    void onWifiDisconnect(const WiFiEventStationModeDisconnected &event);
    void publishMeasurementMessage(String topic, int value);

private:
    Ticker _mqttReconnectTimer;
    MeasurementsMqttMessagePublisher _measurementsMqttClient;
    ConfigurationMqttHandler _initialConfigMqttClient;
    uint16_t _initialConfigPacketIdSub;
    void connectToMqtt();
    void subscribeToMqttEvents();
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
     void onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos);
    void onMqttUnsubscribe(const uint16_t &packetId);
    void onMqttPublish(const uint16_t &packetId);
    void onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties, const size_t &len, const size_t &index, const size_t &total);
    void printSeparationLine();
};

#endif