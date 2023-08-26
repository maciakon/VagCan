#ifndef INTIALCONFIGMQTTCLIENT_H
#define INTIALCONFIGMQTTCLIENT_H_H
#include <AsyncMqttClient_Generic.hpp>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "pids.h"

#define MQTT_HOST IPAddress(18, 185, 185, 121) // 18.185.185.121
#define MQTT_PORT 1883

extern AsyncMqttClient mqttClient;

class InitialConfigMqttClient
{
public:
    activatedPidsKeyValuePair *_activePids;
    const String InitialConfigPubTopic = "remotecardiagz/deviceready";
    const char *InitialConfigSubTopic = "remotecardiagz/initialconfiguration";
    InitialConfigMqttClient(activatedPidsKeyValuePair *activePids);
    void publishInitialConfigurationMessage();
    void handleMessage(char *payload);
private:
    Ticker _mqttReconnectTimer;
};

#endif