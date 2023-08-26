#ifndef INTIALCONFIGMQTTCLIENT_H
#define INTIALCONFIGMQTTCLIENT_H_H
#include <AsyncMqttClient_Generic.hpp>
#include <Ticker.h>
#include <ArduinoJson.h>
#include "can\Pids.h"

#define MQTT_HOST IPAddress(18, 185, 185, 121) // 18.185.185.121
#define MQTT_PORT 1883

extern AsyncMqttClient mqttClient;

class ConfigurationMqttHandler
{
public:
    activatedPidsKeyValuePair *_activePids;
    const String InitialConfigPubTopic = "remotecardiagz/deviceready";
    const char *InitialConfigSubTopic = "remotecardiagz/initialconfiguration";
    const char *MeasurementsChangedSubTopic = "remotecardiagz/activemeasurements";
    ConfigurationMqttHandler(activatedPidsKeyValuePair *activePids);
    void publishInitialConfigurationMessage();
    void handleInitialConfigMessage(char *payload);
    void handleConfigChangedMessage(char *payload);
private:
    Ticker _mqttReconnectTimer;
};

#endif