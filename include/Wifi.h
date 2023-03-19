#ifndef WIFI_H
#define WIFI_H
#include <ESP8266WiFi.h>
#include <Ticker.h>

extern Ticker wifiReconnectTimer;
void onWifiConnect(const WiFiEventStationModeGotIP& event);
#endif
