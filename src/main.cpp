#include <mcp_can.h>
#include <SPI.h>
#include "pids.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#define LISTEN_ID 0x7EA
#define REPLY_ID 0x7E0
#define FUNCTIONAL_ID 0x7DF

// WiFi connection settings
const char* ssid = "VagCan";
const char* password = "vagcan1234";

// Server settings
String serverUrl = "http://51.137.0.132:5001";
String configurationGetEndpoint = "/configuration";
String mesaurementsPostEndpoint ="/measurements";

// Can settings
unsigned long prevTx = 0;
unsigned int invlTx = 1000;
byte txData[] = {0x02,0x01,PID_ENGINE_RPM,0x55,0x55,0x55,0x55,0x55}; // AVAILABLE PIDS
byte activePids[10];

// CAN RX Variables
unsigned long rxID;
byte dlc;
byte rxBuf[8];
char msgString[128];                        // Array to store serial string

// CAN Interrupt and Chip Select Pins
#define CAN0_INT 2                              /* Set INT to pin 2 (This rarely changes)   */
MCP_CAN CAN0(15);                                /* Set CS to pin 15 (Old shields use pin 10) */

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

void receivePID(unsigned char pid)
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

    switch (pid) {
      case PID_COOLANT_TEMP:
        if(rxBuf[2] == PID_COOLANT_TEMP){
          uint8_t temp;
          temp = rxBuf[3] - 40;
          Serial.print("Engine Coolant Temp (degC): ");
          Serial.println(temp, DEC);
        }
      break;

      case PID_ENGINE_RPM:
        if(rxBuf[2] == PID_ENGINE_RPM){
          uint16_t rpm;
          rpm = ((256 * rxBuf[3]) + rxBuf[4]) / 4;
          Serial.print("Engine Speed (rpm): ");
          Serial.println(rpm, DEC);
        }
      break;
    }
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
          for(JsonObject item : doc.as<JsonArray>())
          {
            int i = 0;
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

void setup(){
  Serial.begin(115200);
  while(!Serial);
 
 // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
 
    delay(1000);
    Serial.print("Connecting..");
  }

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
    receivePID(PID_ENGINE_RPM);
  }
 
  /* Every 1000ms (One Second) send a request for PID 00           */
  if((millis() - prevTx) >= invlTx){
    prevTx = millis();
    //sendPID(PID_ENGINE_RPM);
    sendGetRequest(configurationGetEndpoint);
  }
}
