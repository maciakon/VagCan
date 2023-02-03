#include <WString.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

class RemoteCarDiagzAPI 
{
    public:
        RemoteCarDiagzAPI(ESP8266WiFiClass wifi);
        void sendGetRequest(byte* activePids);
        void sendPostMeasurementsRequest(byte* rxBuf);
    private:
        ESP8266WiFiClass _wifi;
        String SERVER_URL;
        String configurationGetEndpoint = "/configuration";
        String mesaurementsPostEndpoint ="/measurements";
};