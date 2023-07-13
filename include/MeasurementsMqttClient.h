#ifndef MEASUREMENTSMQTTCLIENT_H
#define MEASUREMENTSMQTTCLIENT_H
#include <AsyncMqttClient_Generic.hpp>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "pids.h"

#define MQTT_HOST IPAddress(18, 185, 185, 121) // 18.185.185.121
#define MQTT_PORT 1883

extern AsyncMqttClient mqttClient;

class MeasurementsMqttClient
{
public:
    MeasurementsMqttClient(activatedPidsKeyValuePair *activePids);
    void onWifiConnect(const WiFiEventStationModeGotIP &event);
    void onWifiDisconnect(const WiFiEventStationModeDisconnected &event);
    void publishMessage(String topic, byte value);

private:
    Ticker _mqttReconnectTimer;
    activatedPidsKeyValuePair* _activePids;
    void connectToMqtt();
    void subscribeToMqttEvents();
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttSubscribe(const uint16_t &packetId, const uint8_t &qos);
    void onMqttUnsubscribe(const uint16_t &packetId);
    void onMqttMessage(char *topic, char *payload, const AsyncMqttClientMessageProperties &properties, const size_t &len, const size_t &index, const size_t &total);
    void onMqttPublish(const uint16_t &packetId);
    void printSeparationLine();
};

#endif