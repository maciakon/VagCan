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
// WiFi connection settings
const char* WIFI_SSID = "VagCan";
const char* WIFI_PASSWORD = "vagcan1234";

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
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void connectToWifi()
{
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event)
{
  (void) event;

  Serial.print("Connected to Wi-Fi. IP address: ");
  Serial.println(WiFi.localIP());
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event)
{
  (void) event;

  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void setup(){
  Serial.begin(115200);
  while(!Serial);
 
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();

  // Get configuration from server
  RemoteCarDiagzApi.sendGetRequest(activePids);
  // Standard ID Filters
  MPPT.initCan();

  pinMode(CAN0_INT, INPUT);                          // Configuring pin for /INT input
 
  Serial.println("Simple CAN OBD-II PID Request");
}

void loop(){
  if(!digitalRead(CAN0_INT)) {                         // If CAN0_INT pin is low, read receive buffer
    byte* value = MPPT.receivePID();
    RemoteCarDiagzApi.sendPostMeasurementsRequest(value);
  }
 
  /* Every 1000ms (One Second) send a request for PID 00           */
  if((millis() - prevTx) >= invlTx)
  {
    prevTx = millis();
    
    if(activePids[alreadySentIndex] != 0)
    {
      MPPT.sendPID(activePids[alreadySentIndex]);
    }

    alreadySentIndex++;
    if(alreadySentIndex >= 9)
    {
      alreadySentIndex = 0;
    }
  }

  /* Every 5000ms send a request for active PIDs          */
  if((millis() - previousGetActivePids) >= getActivePidsPeriod)
  {
    previousGetActivePids = millis();
    RemoteCarDiagzApi.sendGetRequest(activePids);
  }
}
