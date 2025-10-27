#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "GrpcClient.h"

const String ROVER_AP_SSID = String("MOONBASE-II");
const String ROVER_AP_PASS_PHRASE = String("Trypt1c0n$");

CGrpcClient* grpcClient = nullptr;

// Joystick pin definitions (example pins for ESP32-S3)
#define LEFT_JOYSTICK_X_PIN A0   // GPIO1 (ADC1_CH0)
#define LEFT_JOYSTICK_Y_PIN A1   // GPIO2 (ADC1_CH1)
#define RIGHT_JOYSTICK_X_PIN A2  // GPIO3 (ADC1_CH2)
#define RIGHT_JOYSTICK_Y_PIN A3  // GPIO4 (ADC1_CH3)
#define LEFT_JOYSTICK_BTN_PIN 5  // GPIO5 (digital input with pullup)
#define RIGHT_JOYSTICK_BTN_PIN 6 // GPIO6 (digital input with pullup)

void setup()
{
   // We start by connecting to a WiFi network
   Serial.begin(115200);
   Serial.setDebugOutput(true);
   Serial.println();
   Serial.println("******************************************************");
   log_i("Connecting to %s", ROVER_AP_SSID.c_str());

   WiFi.begin(ROVER_AP_SSID, ROVER_AP_PASS_PHRASE);

   while (WiFi.status() != WL_CONNECTED)
   {
      delay(500);
      Serial.print(".");
   }

   Serial.println("");
   log_i("WiFi connected");
   log_i("IP address: %s", WiFi.localIP().toString().c_str());
   
   IPAddress serverIP = WiFi.gatewayIP();
   log_i("Gateway/Server address: %s", serverIP.toString().c_str());
   
   // Initialize gRPC client
   grpcClient = new CGrpcClient(serverIP, 50051);
   
   // Connect to gRPC server
   if (!grpcClient->Connect())
   {
      log_e("Failed to connect to gRPC server");
      // Could retry or handle error differently
   }
   else
   {
      log_i("Connected to gRPC server successfully");
   }
   
   // Setup joystick pins
   pinMode(LEFT_JOYSTICK_BTN_PIN, INPUT_PULLUP);
   pinMode(RIGHT_JOYSTICK_BTN_PIN, INPUT_PULLUP);
   
   log_i("Joystick pins configured");
   
   // Start IMU data streaming at 20Hz
   delay(1000); // Wait a moment for server to be ready
   GrpcResponse streamResponse = grpcClient->StartImuStreaming(20);
   if (streamResponse.success) {
      log_i("IMU streaming started: %s", streamResponse.data.c_str());
   } else {
      log_e("Failed to start IMU streaming: %s", streamResponse.error.c_str());
   }
}

bool perform_grpc_led_control(bool turnOn)
{
   if (!grpcClient || !grpcClient->IsConnected())
   {
      log_e("gRPC client not connected");
      return false;
   }

   GrpcResponse response;
   if (turnOn)
   {
      response = grpcClient->TurnLedOn();
   }
   else
   {
      response = grpcClient->TurnLedOff();
   }

   if (response.success)
   {
      log_i("LED control successful: %s", response.data.c_str());
      return true;
   }
   else
   {
      log_e("LED control failed: %s", response.error.c_str());
      return false;
   }
}

ImuData get_imu_data()
{
   if (!grpcClient || !grpcClient->IsConnected())
   {
      log_e("gRPC client not connected");
      ImuData emptyData;
      memset(&emptyData, 0, sizeof(ImuData));
      emptyData.valid = false;
      return emptyData;
   }

   return grpcClient->GetAllImuData();
}

void printImuData(const ImuData& imuData) {
   if (!imuData.valid) {
      log_e("Invalid IMU data received");
      return;
   }
   
#ifdef TELEPLOT_ENABLE
   Serial.printf(">AccX:%f\n", imuData.acc_x);
   Serial.printf(">AccY:%f\n", imuData.acc_y);
   Serial.printf(">AccZ:%f\n", imuData.acc_z);
   Serial.printf(">GyroX:%f\n", imuData.gyro_x);
   Serial.printf(">GyroY:%f\n", imuData.gyro_y);
   Serial.printf(">GyroZ:%f\n", imuData.gyro_z);
   Serial.printf(">temp:%f\n", imuData.temperature);
#endif
}

JoystickData readJoystickData()
{
   JoystickData joystickData;
   
   // Read analog values (0-4095 for 12-bit ADC)
   joystickData.left_x = analogRead(LEFT_JOYSTICK_X_PIN);
   joystickData.left_y = analogRead(LEFT_JOYSTICK_Y_PIN);
   joystickData.right_x = analogRead(RIGHT_JOYSTICK_X_PIN);
   joystickData.right_y = analogRead(RIGHT_JOYSTICK_Y_PIN);
   
   // Read button states (inverted because of pullup)
   joystickData.left_button = !digitalRead(LEFT_JOYSTICK_BTN_PIN);
   joystickData.right_button = !digitalRead(RIGHT_JOYSTICK_BTN_PIN);
   
   // Set timestamp
   joystickData.timestamp = millis();
   
   return joystickData;
}

bool sendJoystickData(const JoystickData& joystickData)
{
   if (!grpcClient || !grpcClient->IsConnected())
   {
      log_e("gRPC client not connected");
      return false;
   }

   GrpcResponse response = grpcClient->SendJoystickData(joystickData);
   
   if (response.success)
   {
      log_d("Joystick data sent successfully: %s", response.data.c_str());
      return true;
   }
   else
   {
      log_e("Failed to send joystick data: %s", response.error.c_str());
      return false;
   }
}

void loop()
{
   // Check if gRPC client is still connected
   if (!grpcClient || !grpcClient->IsConnected())
   {
      log_e("gRPC connection lost, attempting to reconnect...");
      if (grpcClient && !grpcClient->Connect())
      {
         log_e("Reconnection failed, waiting before retry...");
         delay(1000);
         return;
      }
   }

   // Read and send joystick data
   JoystickData joystickData = readJoystickData();
   sendJoystickData(joystickData);
   
   // Print joystick data for debugging
   log_d("Joystick: L(%d,%d) R(%d,%d) Btns(L:%d,R:%d)", 
         joystickData.left_x, joystickData.left_y,
         joystickData.right_x, joystickData.right_y,
         joystickData.left_button, joystickData.right_button);
   
   delay(50); // Send joystick data at ~20Hz
   
   // Turn LED on via gRPC (less frequent)
   static unsigned long lastLedToggle = 0;
   if (millis() - lastLedToggle > 500)
   {
      static bool ledState = false;
      perform_grpc_led_control(ledState);
      ledState = !ledState;
      lastLedToggle = millis();
   }
   
   // Process streaming IMU data (continuous)
   ImuData streamingImuData = grpcClient->GetStreamingImuData();
   if (streamingImuData.valid) {
      printImuData(streamingImuData);
      
      // Additional streaming data processing for telemetry
#ifdef TELEPLOT_ENABLE
      Serial.printf(">StreamAccX:%f\n", streamingImuData.acc_x);
      Serial.printf(">StreamAccY:%f\n", streamingImuData.acc_y);
      Serial.printf(">StreamAccZ:%f\n", streamingImuData.acc_z);
      Serial.printf(">StreamGyroX:%f\n", streamingImuData.gyro_x);
      Serial.printf(">StreamGyroY:%f\n", streamingImuData.gyro_y);
      Serial.printf(">StreamGyroZ:%f\n", streamingImuData.gyro_z);
      Serial.printf(">StreamTemp:%f\n", streamingImuData.temperature);
#endif
   }
}
