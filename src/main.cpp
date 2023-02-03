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

//my mppt setup
VagCanMptt MPPT(15);
RemoteCarDiagzAPI RemoteCarDiagzApi(WiFi);

// MQTT Protocol setup
#define MQTT_HOST         IPAddress(3, 71, 12 , 183)
// #define MQTT_HOST         "broker.emqx.io"        // Broker address
#define MQTT_PORT         1883

const char *PubTopic  = "async-mqtt/ESP8266_Pub";
const char *SubTopic  = "remotecardiagz/activemeasurements";
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

void connectToMqtt()
{
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
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

void printSeparationLine()
{
  Serial.println("************************************************");
}

void onMqttConnect(bool sessionPresent)
{
  Serial.print("Connected to MQTT broker: ");
  Serial.print(MQTT_HOST);
  Serial.print(", port: ");
  Serial.println(MQTT_PORT);
  Serial.print("PubTopic: ");
  Serial.println(PubTopic);

  printSeparationLine();
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  uint16_t packetIdSub = mqttClient.subscribe(SubTopic, 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);

  // uint16_t packetIdPub1 = mqttClient.publish(PubTopic, 1, true, "ESP8266 Test2");
  // Serial.print("Publishing at QoS 1, packetId: ");
  // Serial.println(packetIdPub1);

  printSeparationLine();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  (void) reason;

  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected())
  {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(const uint16_t& packetId, const uint8_t& qos)
{
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(const uint16_t& packetId)
{
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, const AsyncMqttClientMessageProperties& properties,
                   const size_t& len, const size_t& index, const size_t& total)
{
  (void) payload;

  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
  Serial.println("    payload:    ");
  Serial.print(payload);
}

void onMqttPublish(const uint16_t& packetId)
{
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup(){
  Serial.begin(115200);
  while(!Serial);
 
 // Initialize WiFi
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
 
  //   delay(1000);
  //   Serial.print("Connecting...");
  // }

 // ****************************
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
