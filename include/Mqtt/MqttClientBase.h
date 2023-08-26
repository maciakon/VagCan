#ifndef MQTTCLIENTBASE_H
#define MQTTCLIENTBASE_H
#include <AsyncMqttClient_Generic.hpp>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "pids.h"
#include "InitialConfigMqttClient.h"
#include "MeasurementsMqttClient.h"

#define MQTT_HOST IPAddress(18, 185, 185, 121) // 18.185.185.121
#define MQTT_PORT 1883

extern AsyncMqttClient mqttClient;

class MqttClientBase
{
public:
    activatedPidsKeyValuePair *_activePids;
    MqttClientBase(activatedPidsKeyValuePair *activePids);
    void onWifiConnect(const WiFiEventStationModeGotIP &event);
    void onWifiDisconnect(const WiFiEventStationModeDisconnected &event);
    void publishMeasurementMessage(String topic, byte value);

private:
    Ticker _mqttReconnectTimer;
    MeasurementsMqttClient _measurementsMqttClient;
    InitialConfigMqttClient _initialConfigMqttClient;
    uint16_t _initialConfigPacketIdSub;
    void connectToMqtt();
    void subscribeToMqttEvents();
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttUnsubscribe(const uint16_t &packetId);
    void onMqttPublish(const uint16_t &packetId);
    void printSeparationLine();

protected:
    virtual void onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties, const size_t &len, const size_t &index, const size_t &total);
    virtual void onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos);
};

#endif