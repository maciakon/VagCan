#ifndef MEASUREMENTSMQTTCLIENT_H
#define MEASUREMENTSMQTTCLIENT_H
#include <AsyncMqttClient_Generic.hpp>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "Can\Pids.h"

extern AsyncMqttClient mqttClient;

class MeasurementsMqttMessagePublisher
{
public:
    const String PubTopic = "remotecardiagz/pids/";
    MeasurementsMqttMessagePublisher(activatedPidsKeyValuePair *activePids);
    void publishMessage(String topic, uint16_t value);

private:
    Ticker _mqttReconnectTimer;
    activatedPidsKeyValuePair *_activePids;
};

#endif