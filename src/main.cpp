#include <mcp_can.h>
#include <SPI.h>
#include "pids.h"
#include "VagCanMppt.h"
#include "RemoteCarDiagzApi.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <AsyncMqtt_Generic.h>
#include "Mqtt.h"
#include "Wifi.h"

unsigned long previousGetActivePids = 0;
unsigned int getActivePidsPeriod = 5000;

// Can settings
unsigned long prevTx = 0;
unsigned int invlTx = 200;
byte activePids[10];
int alreadySentIndex = 0;

VagCanMptt MPPT(15);
RemoteCarDiagzAPI RemoteCarDiagzApi(WiFi);
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

void setup()
{
  Serial.begin(115200);
  while (!Serial);

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  subscribeToMqttEvents();
  connectToWifi();

  // Get configuration from server
  RemoteCarDiagzApi.sendGetRequest(activePids);

  // Standard ID Filters
  MPPT.initCan();

  pinMode(CAN0_INT, INPUT); // Configuring pin for /INT input

  Serial.println("Simple CAN OBD-II PID Request");
}

void loop()
{
  if (!digitalRead(CAN0_INT))
  { // If CAN0_INT pin is low, read receive buffer
    byte *value = MPPT.receivePID();
    RemoteCarDiagzApi.sendPostMeasurementsRequest(value);
  }

  /* Every 1000ms (One Second) send a request for PID 00           */
  if ((millis() - prevTx) >= invlTx)
  {
    prevTx = millis();

    if (activePids[alreadySentIndex] != 0)
    {
      MPPT.sendPID(activePids[alreadySentIndex]);
    }

    alreadySentIndex++;
    if (alreadySentIndex >= 9)
    {
      alreadySentIndex = 0;
    }
  }

  /* Every 5000ms send a request for active PIDs          */
  if ((millis() - previousGetActivePids) >= getActivePidsPeriod)
  {
    previousGetActivePids = millis();
    RemoteCarDiagzApi.sendGetRequest(activePids);
  }
}
