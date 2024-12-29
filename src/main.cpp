#include <Arduino.h>

#include <WiFi.h>

#include <HTTPClient.h>
#include <ArduinoJson.h>

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

void perform_http_get(String hostUrl, String& contents)
{
   // wait for WiFi connection
   if (WiFi.status() == WL_CONNECTED)
   {
      HTTPClient http;

      log_i("[HTTP] begin...");

      http.begin(hostUrl.c_str()); // HTTP

      log_i("[HTTP] GET...");
      // start connection and send HTTP header
      int httpCode = http.GET();

      // httpCode will be negative on error
      if (httpCode > 0)
      {
         // HTTP header has been send and Server response header has been handled
         log_i("[HTTP] GET... code: %d", httpCode);

         // file found at server
         if (httpCode == HTTP_CODE_OK)
         {
            String payload = http.getString();
#ifdef PRINT_HTTP_CONTENT
            Serial.println("Payload: ");
            Serial.println(payload);
#endif
            contents=payload;
         }
      }
      else
      {
         log_e("[HTTP] GET... failed, error: %s", http.errorToString(httpCode).c_str());
      }

      http.end();
   }
}


void perform_http_get(String hostUrl)
{
   String contents;
   perform_http_get(hostUrl, contents);
}

void printJSONContents(String jsonString) {
   if (jsonString.length() == 0 ){
      return;
   }
   JsonDocument doc;
   DeserializationError error = deserializeJson(doc, jsonString);
   if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
   }
#ifdef ENABLE_TELEPLOT
   Serial.printf(">AccX:%f\n", doc["accX"].as<float>());
   Serial.printf(">AccY:%f\n", doc["accY"].as<float>());
   Serial.printf(">AccZ:%f\n", doc["accZ"].as<float>());
   Serial.printf(">GyroX:%f\n", doc["gyroX"].as<float>());
   Serial.printf(">GyroY:%f\n", doc["gyroY"].as<float>());
   Serial.printf(">GyroZ:%f\n", doc["gyroZ"].as<float>());
   Serial.printf(">temp:%f\n", doc["temperature"].as<float>());
#endif
}

void loop()
{
   String hostUrl = String("http://") + hostname;
   String commanOndUrl = hostUrl + String("/led-on");
   perform_http_get(commanOndUrl);
   delay(75);
   String commandOffUrl = hostUrl + String("/led-off");
   perform_http_get(commandOffUrl);
   delay(75);
   String imudataUrl = hostUrl + String("/all-imu-data");
   String contents;
   perform_http_get(imudataUrl, contents);
   // Serial.println(contents);
   printJSONContents(contents);
   delay(75);
}
