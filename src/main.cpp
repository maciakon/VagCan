#include <mcp_can.h>
#include <SPI.h>
#include "pids.h"
#include "VagCanMCP.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <AsyncMqtt_Generic.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "Mqtt\MqttClientWrapper.h"
#include "Wifi.h"

activatedPidsKeyValuePair activePids[10];
byte lastPidResponseReceived = 0x00;
byte lastPidRequestSent = 0x00;
uint64 alreadySentIndex = 0;
AsyncMqttClient mqttClient;
VagCanMCP VagMCP(15);
Ticker wifiReconnectTimer;
Ticker sendPidRequestTimer;
MqttClientWrapper mqttClientWrapper(activePids);
WiFiEventHandler mqttClientWifiHandler;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
keyValuePair calculateValue(byte *sensorReading);
void setupWifiConnectionHandlers();
void sendPidRequest();

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  setupWifiConnectionHandlers();
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  VagMCP.initCan();
  sendPidRequestTimer.attach_ms(200, sendPidRequest);
}

void loop()
{
  if (!digitalRead(CAN0_INT))
  {
    byte *value = VagMCP.receivePID(); // If CAN0_INT pin is low, read receive buffer
    keyValuePair kvp = calculateValue(value);
    mqttClientWrapper.publishMeasurementMessage(kvp.humanReadable, kvp.value);
    lastPidResponseReceived = value[2];
  }
}

void sendPidRequest()
{
  if (lastPidRequestSent == lastPidResponseReceived) // make sure we're ready to send another PID request
  {
    if (activePids[alreadySentIndex].isActive == true)
    {
      Serial.println("Send PID request.");
      VagMCP.sendPID(activePids[alreadySentIndex].pidId);
      lastPidRequestSent = activePids[alreadySentIndex].pidId;
    }
    alreadySentIndex++;
    if (alreadySentIndex > sizeof(activePids) / sizeof(activatedPidsKeyValuePair) - 1)
    {
      alreadySentIndex = 0;
    }
  }
}

void setupWifiConnectionHandlers()
{
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  mqttClientWifiHandler = WiFi.onStationModeGotIP(std::bind(&MqttClientWrapper::onWifiConnect, mqttClientWrapper, std::placeholders::_1));
}
