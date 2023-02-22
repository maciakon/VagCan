#include "Wifi.h"

// WiFi connection settings
const char *WIFI_SSID = "VagCan";
const char *WIFI_PASSWORD = "vagcan1234";

void connectToWifi()
{
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP &event)
{
    (void)event;

    Serial.print("Connected to Wi-Fi. IP address: ");
    Serial.println(WiFi.localIP());
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected &event)
{
    (void)event;

    Serial.println("Disconnected from Wi-Fi.");
    wifiReconnectTimer.once(2, connectToWifi);
}