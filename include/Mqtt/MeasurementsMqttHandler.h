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

class MeasurementsMqttHandler
{
public:
    const String PubTopic = "remotecardiagz/pids/";
    const char *SubTopic = "remotecardiagz/activemeasurements";
    MeasurementsMqttHandler(activatedPidsKeyValuePair *activePids);
    void publishMessage(String topic, byte value);
    void handleMessage(char *payload);

private:
    
    Ticker _mqttReconnectTimer;
    activatedPidsKeyValuePair *_activePids;
};

#endif