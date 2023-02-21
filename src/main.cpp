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

// Can settings
unsigned long prevTx = 0;
unsigned int invlTx = 200;
byte activePids[10];
int alreadySentIndex = 0;

VagCanMCP VagMCP(15);
RemoteCarDiagzAPI RemoteCarDiagzApi(activePids);
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler mcpWifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  mcpWifiConnectHandler = WiFi.onStationModeGotIP(std::bind(&RemoteCarDiagzAPI::onWifiConnect, RemoteCarDiagzApi,std::placeholders::_1));
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  subscribeToMqttEvents();
  connectToWifi();

  VagMCP.initCan();   // Initialize CAN

  pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input
}

void loop()
{
  if (!digitalRead(CAN0_INT))
  { // If CAN0_INT pin is low, read receive buffer
    byte *value = VagMCP.receivePID();
    RemoteCarDiagzApi.sendPostMeasurementsRequest(value);
    // here goes MQTT publish
  }

  /* Every 1000ms (One Second) send a request for PID 00           */
  if ((millis() - prevTx) >= invlTx)
  {
    prevTx = millis();

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
}
