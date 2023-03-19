#include "Wifi.h"

// WiFi connection settings
const char *WIFI_SSID = "VagCan";
const char *WIFI_PASSWORD = "vagcan1234";

void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
    (void)event;

    Serial.print("Connected to Wi-Fi. IP address: ");
    Serial.println(WiFi.localIP());
}