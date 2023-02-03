#include <mcp_can.h>
#include <SPI.h>
#include "pids.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <AsyncMqtt_Generic.h>

#define LISTEN_ID 0x7EA
#define REPLY_ID 0x7E0
#define FUNCTIONAL_ID 0x7DF

// WiFi connection settings
const char* WIFI_SSID = "VagCan";
const char* WIFI_PASSWORD = "vagcan1234";

// Server settings
String serverUrl = "http://20.76.183.147:5001";
String configurationGetEndpoint = "/configuration";
String mesaurementsPostEndpoint ="/measurements";
unsigned long previousGetActivePids = 0;
unsigned int getActivePidsPeriod = 5000;


// Can settings
unsigned long prevTx = 0;
unsigned int invlTx = 200;
byte txData[] = {0x02,0x01,PID_ENGINE_RPM,0x55,0x55,0x55,0x55,0x55}; // AVAILABLE PIDS
byte activePids[10];
int alreadySentIndex = 0;

// CAN RX Variables
unsigned long rxID;
byte dlc;
byte rxBuf[8];
char msgString[128];                        // Array to store serial string

// CAN Interrupt and Chip Select Pins
#define CAN0_INT 2                              /* Set INT to pin 2 (This rarely changes)   */
MCP_CAN CAN0(15);                                /* Set CS to pin 15 (Old shields use pin 10) */

// MQTT Protocol setup

#define MQTT_HOST         IPAddress(127, 0, 0 , 1)
// #define MQTT_HOST         "broker.emqx.io"        // Broker address
#define MQTT_PORT         1883

const char *PubTopic  = "async-mqtt/ESP8266_Pub";
const char *SubTopic  = "remotecardiagz/activemeasurements";
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void sendPostMeasurementsRequest(String endpoint, byte rxBuf[]);

void sendPID(unsigned char pid)
{
  unsigned char txData[8] = {0x02, 0x01, pid, 0, 0, 0, 0, 0};

  byte sndStat = CAN0.sendMsgBuf(FUNCTIONAL_ID, 0, 8, txData);

  if (sndStat == CAN_OK) {
    Serial.print("PID sent: 0x");
    Serial.println(pid, HEX);
  }
  else {
    Serial.println("Error Sending Message...");
  }
}

void receivePID()
{
    CAN0.readMsgBuf(&rxID, &dlc, rxBuf);      // Read data: len = data length, buf = data byte(s)

    // Display received CAN data as we receive it.
    sprintf(msgString, "Standard ID: 0x%.3lX, DLC: %1d, Data: ", rxID, dlc);
    Serial.print(msgString);

    for (byte i = 0; i < dlc; i++) {
      sprintf(msgString, " 0x%.2X", rxBuf[i]);
      Serial.print(msgString);
    }
    Serial.println("");

    sendPostMeasurementsRequest(mesaurementsPostEndpoint, rxBuf);
}

void sendGetRequest(String endpoint)
{
   StaticJsonDocument<768> doc;
   if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      String serverPath = serverUrl + endpoint;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());
      
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        int str_len = payload.length() + 1; 
        // Prepare the character array (the buffer) 
        char char_array[str_len];

        // Copy it over 
        payload.toCharArray(char_array, str_len);
        Serial.println(payload);

        DeserializationError error = deserializeJson(doc, char_array);
        if (error) 
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
        }
        else
        {
          // clear activePids array
          for(int i = 0; i < 10; i++)
          {
            activePids[i] = 0;
          }

          int i = 0;
          // write active pids to array
          for(JsonObject item : doc.as<JsonArray>())
          {
            byte pidValue = item["value"]; 
            bool isActive = item["isActive"];
            if(isActive)
            {
              activePids[i] = pidValue;
              Serial.print("Adding active PID value: ");
              Serial.println(pidValue, DEC);
              i++;
            }
          }
        }
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
}

void sendPostMeasurementsRequest(String endpoint, byte rxBuf[])
{
   if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      String serverPath = serverUrl + endpoint;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());
      http.addHeader("Content-Type", "application/json");
      StaticJsonDocument<96> doc;

      doc["PIDCode"] = rxBuf[2];
      doc["A"] = rxBuf[3];
      doc["B"] = rxBuf[4];
      doc["C"] = rxBuf[5];
      doc["D"] = rxBuf[6];

      char output[128];
      serializeJson(doc, output);
      Serial.print("Sending following json:");
      Serial.println(output);

      int httpResponseCode = http.POST(output);
      
      if (httpResponseCode > 0) 
      {
        Serial.print("POST HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
   }
}

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

  uint16_t packetIdSub = mqttClient.subscribe(PubTopic, 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);

  mqttClient.publish(PubTopic, 0, true, "ESP8266 Test1");
  Serial.println("Publishing at QoS 0");

  uint16_t packetIdPub1 = mqttClient.publish(PubTopic, 1, true, "ESP8266 Test2");
  Serial.print("Publishing at QoS 1, packetId: ");
  Serial.println(packetIdPub1);

  uint16_t packetIdPub2 = mqttClient.publish(PubTopic, 2, true, "ESP8266 Test3");
  Serial.print("Publishing at QoS 2, packetId: ");
  Serial.println(packetIdPub2);

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
 // ****************************
  // Get configuration from server
  sendGetRequest(configurationGetEndpoint);

  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_NORMAL, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else{
    Serial.println("Error Initializing MCP2515... Permanent failure!  Check your code & connections");
    while(1);
  }

  // Standard ID Filters
  CAN0.init_Mask(0,0x7F00000);                // Init first mask...
  CAN0.init_Filt(0,0x7DF0000);                // Init first filter...
  CAN0.init_Filt(1,0x7E10000);                // Init second filter...

  CAN0.init_Mask(1,0x7F00000);                // Init second mask...
  CAN0.init_Filt(2,0x7DF0000);                // Init third filter...
  CAN0.init_Filt(3,0x7E10000);                // Init fouth filter...
  CAN0.init_Filt(4,0x7DF0000);                // Init fifth filter...
  CAN0.init_Filt(5,0x7E10000);                // Init sixth filter...

  CAN0.setMode(MCP_LOOPBACK);                      // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                          // Configuring pin for /INT input
 
  Serial.println("Simple CAN OBD-II PID Request");
}

void loop(){

  if(!digitalRead(CAN0_INT)) {                         // If CAN0_INT pin is low, read receive buffer
    receivePID();
  }
 
  /* Every 1000ms (One Second) send a request for PID 00           */
  if((millis() - prevTx) >= invlTx)
  {
    prevTx = millis();
    
    if(activePids[alreadySentIndex] != 0)
    {
      sendPID(activePids[alreadySentIndex]);
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
    sendGetRequest(configurationGetEndpoint);
  }
}
