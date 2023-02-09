#ifndef MQTT_H
#define MQTT_H
#include <AsyncMqttClient_Generic.hpp>
#include <ESP8266WiFi.h>
#include <Ticker.h>
// MQTT Protocol setup
#define MQTT_HOST         IPAddress(18, 185, 185 , 121) // 18.185.185.121
// #define MQTT_HOST         "broker.emqx.io"        // Broker address
#define MQTT_PORT         1883

extern AsyncMqttClient mqttClient;
extern Ticker mqttReconnectTimer;

void connectToMqtt();
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void onMqttSubscribe(const uint16_t& packetId, const uint8_t& qos);
void onMqttUnsubscribe(const uint16_t& packetId);
void onMqttMessage(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties, const size_t& len, const size_t& index, const size_t& total);
void onMqttPublish(const uint16_t& packetId);
void printSeparationLine();
#endif