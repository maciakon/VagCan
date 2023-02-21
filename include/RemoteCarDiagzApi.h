#ifndef REMOTE_CAR_DIAGZ_API_H
#define REMOTE_CAR_DIAGZ_API_H
#include <WString.h>
#include <ESP8266WiFi.h>
#include <AsyncHTTPRequest_Generic.hpp>
#include <ArduinoJson.h>

class RemoteCarDiagzAPI 
{
    public:
        RemoteCarDiagzAPI(byte* activePids);
        void onWifiConnect(const WiFiEventStationModeGotIP &event);
        void sendPostMeasurementsRequest(byte* rxBuf);
        static void requestCB(void *optParm, AsyncHTTPRequest *request, int readyState);
        
    private:
        AsyncHTTPRequest _request;
        String SERVER_URL;
        String configurationGetEndpoint = "/configuration";
        String mesaurementsPostEndpoint ="/measurements";
};
#endif