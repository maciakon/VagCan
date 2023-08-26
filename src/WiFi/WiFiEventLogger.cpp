#include "WiFi\WifiEventLogger.h"

void WifiEventLogger::onWifiConnect(const WiFiEventStationModeGotIP &event)
{
    (void)event;
    Serial.print("Connected to Wi-Fi. IP address: ");
    Serial.println(WiFi.localIP());
}
