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
#include "Mqtt.h"
#include "Wifi.h"

unsigned long previousGetActivePids = 0;
unsigned int getActivePidsPeriod = 5000;
byte activePids[10];
int alreadySentIndex = 0;
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

void setupWifiConnectionHandlers();
void sendPidRequest();

void setup()
{
  Serial.begin(115200);
  while (!Serial);
  setupWifiConnectionHandlers();
  connectToWifi();
  VagMCP.initCan();   // Initialize CAN
  sendPidRequestTimer.attach_ms(500, sendPidRequest);
}

void loop()
{
  if (!digitalRead(CAN0_INT))
  { 
    byte *value = VagMCP.receivePID();  // If CAN0_INT pin is low, read receive buffer
    RemoteCarDiagzApi.sendPostMeasurementsRequest(value);
    // here goes MQTT publish
  }
}

void sendPidRequest()
{
   if (activePids[alreadySentIndex] != 0)
    {
      VagMCP.sendPID(activePids[alreadySentIndex]);
    }

    alreadySentIndex++;
    if (alreadySentIndex >= 9)
    {
      alreadySentIndex = 0;
    }
}

void setupWifiConnectionHandlers()
{
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  mcpWifiConnectHandler = WiFi.onStationModeGotIP(std::bind(&RemoteCarDiagzAPI::onWifiConnect, RemoteCarDiagzApi,std::placeholders::_1));
  mqttWifiConnectHandler = WiFi.onStationModeGotIP(std::bind(&Mqtt::onWifiConnect, RemoteCarDiagzMqtt,std::placeholders::_1));
  mqttWifiDisconnectHandler =  WiFi.onStationModeDisconnected(std::bind(&Mqtt::onWifiDisconnect, RemoteCarDiagzMqtt,std::placeholders::_1));
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
}
