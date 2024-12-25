#include <Arduino.h>

#include <WiFi.h>

#include <HTTPClient.h>

const String ROVER_AP_SSID = String("MOONBASE-II");
const String ROVER_AP_PASS_PHRASE = String("Trypt1c0n$");

String hostname;

void setup()
{

   // We start by connecting to a WiFi network
   Serial.begin(115200);
   Serial.setDebugOutput(true);
   Serial.println();
   Serial.println("******************************************************");
   log_i("Connecting to ");
   Serial.println(ROVER_AP_SSID.c_str());

   WiFi.begin(ROVER_AP_SSID, ROVER_AP_PASS_PHRASE);

   while (WiFi.status() != WL_CONNECTED)
   {
      delay(500);
      Serial.print(".");
   }

   Serial.println("");
   log_i("WiFi connected");
   log_i("IP address: %s",WiFi.localIP().toString().c_str());
   
   hostname = WiFi.gatewayIP().toString();
   log_i("Gatway address: %s", hostname.c_str());
}

void perform_http_get(String hostUrl)
{
   // wait for WiFi connection
   if (WiFi.status() == WL_CONNECTED)
   {
      HTTPClient http;

      log_i("[HTTP] begin...\n");

      http.begin(hostUrl.c_str()); // HTTP

      log_i("[HTTP] GET...\n");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
         // HTTP header has been send and Server response header has been handled
         log_i("[HTTP] GET... code: %d\n", httpCode);

         // file found at server
         if (httpCode == HTTP_CODE_OK)
         {
            String payload = http.getString();
            Serial.println("Payload: ");
            Serial.println(payload);
         }
      }
      else
      {
         log_e("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
   }
}

void loop()
{
   String hostUrl = String("http://") + hostname;
   String commanOndUrl = hostUrl + String("/on");
   perform_http_get(commanOndUrl);
   delay(250);
   String commandOffUrl = hostUrl + String("/off");
   perform_http_get(commandOffUrl);
   delay(250);
}
