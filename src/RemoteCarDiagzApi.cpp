#include "RemoteCarDiagzApi.h"

RemoteCarDiagzAPI::RemoteCarDiagzAPI(byte *activePids)
{
  _request.onReadyStateChange(requestCB, activePids);
  SERVER_URL = "http://18.185.185.121:5001";
}

void RemoteCarDiagzAPI::onWifiConnect(const WiFiEventStationModeGotIP &event)
{
  (void)event;
  String serverPath = SERVER_URL + configurationGetEndpoint;
  Serial.println("API WIFI CONNECTED");
  static bool requestOpenResult;
  if (_request.readyState() == readyStateUnsent || _request.readyState() == readyStateDone)
  {
    requestOpenResult = _request.open("GET", serverPath.c_str());

    if (requestOpenResult)
    {
      // Only send() if open() returns true, or crash
      _request.send();
    }
    else
    {
      Serial.println("Can't send bad request");
    }
  }
  else
  {
    Serial.println("Can't send request");
  }
}

void RemoteCarDiagzAPI::requestCB(void *optParm, AsyncHTTPRequest *request, int readyState)
{
  byte *activePids = (byte *)optParm;
  StaticJsonDocument<768> doc;
  if (readyState == readyStateDone)
  {
    AHTTP_LOGDEBUG(F("\n**************************************"));
    AHTTP_LOGDEBUG1(F("Response Code = "), request->responseHTTPString());

    if (request->responseHTTPcode() == 200)
    {
      String payload = request->responseText();
      Serial.println(F("\n**************************************"));
      Serial.println(payload);
      Serial.println(F("**************************************"));

      int str_len = payload.length() + 1;
      // Prepare the character array (the buffer)
      char char_array[str_len];

      // Copy it over
      payload.toCharArray(char_array, str_len);
      DeserializationError error = deserializeJson(doc, char_array);
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
      }
      else
      {
        // clear activePids array
        for (int i = 0; i < 10; i++)
        {
          activePids[i] = 0;
        }

        int i = 0;
        // write active pids to array
        for (JsonObject item : doc.as<JsonArray>())
        {
          byte pidValue = item["value"];
          bool isActive = item["isActive"];
          if (isActive)
          {
            activePids[i] = pidValue;
            Serial.print("Adding active PID value: ");
            Serial.println(pidValue, DEC);
            i++;
          }
        }
      }
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(request->responseHTTPString());
    }
  }
}