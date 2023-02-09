#ifndef WIFI_H
#define WIFI_H
#include <ESP8266WiFi.h>
#include <Ticker.h>

extern Ticker wifiReconnectTimer;
extern Ticker mqttReconnectTimer;
void connectToWifi();
void onWifiConnect(const WiFiEventStationModeGotIP& event);
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event);
#endif
