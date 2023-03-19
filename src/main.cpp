#include <mcp_can.h>
#include <SPI.h>
#include "pids.h"
#include "VagCanMCP.h"
#include "RemoteCarDiagzApi.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <AsyncMqtt_Generic.h>
#include <AsyncHTTPRequest_Generic.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>  
#include "Mqtt.h"
#include "Wifi.h"

byte activePids[10];
byte lastPidResponseReceived = 0x00;
byte lastPidRequestSent = 0x00;
uint64 alreadySentIndex = 0;
AsyncMqttClient mqttClient;
VagCanMCP VagMCP(15);
RemoteCarDiagzAPI RemoteCarDiagzApi(activePids);
Mqtt RemoteCarDiagzMqtt(activePids);
Ticker wifiReconnectTimer;
Ticker sendPidRequestTimer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler mqttWifiConnectHandler;
WiFiEventHandler mqttWifiDisconnectHandler;
WiFiEventHandler mcpWifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
keyValuePair calculateValue(byte *sensorReading);
void setupWifiConnectionHandlers();
void sendPidRequest();

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  setupWifiConnectionHandlers();
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  //connectToWifi();
  VagMCP.initCan();
  sendPidRequestTimer.attach_ms(50, sendPidRequest);
}

void loop()
{
  if (!digitalRead(CAN0_INT))
  {
    byte *value = VagMCP.receivePID(); // If CAN0_INT pin is low, read receive buffer
    keyValuePair kvp = calculateValue(value);
    RemoteCarDiagzMqtt.publishMessage(kvp.humanReadable, kvp.value);
    lastPidResponseReceived = value[2];
  }
}

void sendPidRequest()
{
  if (lastPidRequestSent == lastPidResponseReceived) //make sure we're ready to send another PID request
  {
    if (activePids[alreadySentIndex] != 0)
    {
      VagMCP.sendPID(activePids[alreadySentIndex]);
      lastPidRequestSent = activePids[alreadySentIndex];
    }
    alreadySentIndex++;
    if (alreadySentIndex > sizeof(activePids) / sizeof(byte) - 1)
    {
      alreadySentIndex = 0;
    }
  }
}

void setupWifiConnectionHandlers()
{
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  mcpWifiConnectHandler = WiFi.onStationModeGotIP(std::bind(&RemoteCarDiagzAPI::onWifiConnect, RemoteCarDiagzApi, std::placeholders::_1));
  mqttWifiConnectHandler = WiFi.onStationModeGotIP(std::bind(&Mqtt::onWifiConnect, RemoteCarDiagzMqtt, std::placeholders::_1));
  mqttWifiDisconnectHandler = WiFi.onStationModeDisconnected(std::bind(&Mqtt::onWifiDisconnect, RemoteCarDiagzMqtt, std::placeholders::_1));
}
