#include "RemoteCarDiagzApi.h"

RemoteCarDiagzAPI::RemoteCarDiagzAPI(ESP8266WiFiClass wifi)
{
  _wifi = wifi;
  SERVER_URL = "http://18.185.185.121:5001";
}

void RemoteCarDiagzAPI::sendGetRequest(byte* activePids)
{
  StaticJsonDocument<768> doc;

  if(_wifi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;

    String serverPath = SERVER_URL + configurationGetEndpoint;
    
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
        // clear activePids array
        for(int i = 0; i < 10; i++)
        {
          activePids[i] = 0;
        }

        int i = 0;
        // write active pids to array
        for(JsonObject item : doc.as<JsonArray>())
        {
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

void RemoteCarDiagzAPI::sendPostMeasurementsRequest(byte* rxBuf)
{
   if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http;

      String serverPath = SERVER_URL + mesaurementsPostEndpoint;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());
      http.addHeader("Content-Type", "application/json");
      StaticJsonDocument<96> doc;

      doc["PIDCode"] = rxBuf[2];
      doc["A"] = rxBuf[3];
      doc["B"] = rxBuf[4];
      doc["C"] = rxBuf[5];
      doc["D"] = rxBuf[6];

      char output[128];
      serializeJson(doc, output);
      Serial.print("Sending following json:");
      Serial.println(output);

      int httpResponseCode = http.POST(output);
      
      if (httpResponseCode > 0) 
      {
        Serial.print("POST HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
   }
}
