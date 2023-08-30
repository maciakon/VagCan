#include <mcp_can.h>
#include <SPI.h>
#include "Can\Pids.h"
#include "Can\PidProcessor.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <AsyncMqtt_Generic.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "Mqtt\MqttClientWrapper.h"
#include "Wifi\WifiEventLogger.h"
#include "Can\MeasurementValueCalculator.h"

activatedPidsKeyValuePair activePids[10];
byte lastPidResponseReceived = 0x00;
byte lastPidRequestSent = 0x00;
uint64 alreadySentIndex = 0;
AsyncMqttClient mqttClient;
PidProcessor pidProcessing(15);
MeasurementValueCalculator measurementValueCalculations;
Ticker wifiReconnectTimer;
Ticker sendPidRequestTimer;
WifiEventLogger wifiEventLogger;
MqttClientWrapper mqttClientWrapper(activePids);
WiFiEventHandler mqttClientWifiHandler;
WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
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
  pidProcessing.initCan();
  sendPidRequestTimer.attach_ms(200, sendPidRequest);
}

void loop()
{
  if (!digitalRead(CAN0_INT))
  {
    byte *value = pidProcessing.receivePID(); // If CAN0_INT pin is low, read receive buffer
    humanReadablePidValue kvp = measurementValueCalculations.calculateValueFromSensorReading(value);
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
      pidProcessing.sendPID(activePids[alreadySentIndex].pidId);
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
  wifiConnectHandler = WiFi.onStationModeGotIP(std::bind(&WifiEventLogger::onWifiConnect, wifiEventLogger, std::placeholders::_1));
  mqttClientWifiHandler = WiFi.onStationModeGotIP(std::bind(&MqttClientWrapper::onWifiConnect, mqttClientWrapper, std::placeholders::_1));
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(std::bind(&MqttClientWrapper::onWifiDisconnect, mqttClientWrapper, std::placeholders::_1));
}
