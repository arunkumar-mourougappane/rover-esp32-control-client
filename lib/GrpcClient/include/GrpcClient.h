/**
 * @file GrpcClient.h
 * @author Arunkumar Mourougappane (amouroug@buffalo.edu)
 * @brief gRPC client implementation for ESP32 rover control
 * @version 1.0.0
 * @date 2025-10-26
 * 
 * Copyright (c) Arunkumar Mourougappane
 * 
 */

#ifndef GRPC_CLIENT_H
#define GRPC_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

// Response structure for gRPC calls
struct GrpcResponse {
    bool success;
    String data;
    String error;
    int64_t timestamp;
};

// IMU data structure for easy parsing
struct ImuData {
    float acc_x, acc_y, acc_z;
    float gyro_x, gyro_y, gyro_z;
    float temperature;
    int64_t timestamp;
    bool valid;
};

// Joystick data structure for control input
struct JoystickData {
    // Left joystick analog values (0-4095 for 12-bit ADC)
    int left_x;     // Left joystick X axis
    int left_y;     // Left joystick Y axis
    
    // Right joystick analog values (0-4095 for 12-bit ADC)
    int right_x;    // Right joystick X axis
    int right_y;    // Right joystick Y axis
    
    // Optional button states (if joysticks have push buttons)
    bool left_button;  // Left joystick button pressed
    bool right_button; // Right joystick button pressed
    
    // Metadata
    int64_t timestamp; // Timestamp when data was captured
};

class CGrpcClient {
public:
    /**
     * @brief Construct a new CGrpcClient object
     * 
     * @param serverIP Server IP address
     * @param serverPort Server port (default 50051)
     */
    CGrpcClient(IPAddress serverIP, int serverPort = 50051);
    
    /**
     * @brief Connect to the gRPC server
     * 
     * @return true if connection successful
     */
    bool Connect();
    
    /**
     * @brief Disconnect from the gRPC server
     */
    void Disconnect();
    
    /**
     * @brief Check if client is connected to server
     * 
     * @return true if connected
     */
    bool IsConnected();
    
    /**
     * @brief Turn LED on via gRPC call
     * 
     * @return GrpcResponse Response from server
     */
    GrpcResponse TurnLedOn();
    
    /**
     * @brief Turn LED off via gRPC call
     * 
     * @return GrpcResponse Response from server
     */
    GrpcResponse TurnLedOff();
    
    /**
     * @brief Get all IMU data via gRPC call
     * 
     * @return ImuData Complete IMU sensor data
     */
    ImuData GetAllImuData();
    
    /**
     * @brief Get specific IMU data parameter
     * 
     * @param parameter Parameter to retrieve ("acc", "gyro", "accx", etc.)
     * @return ImuData Requested IMU data (other fields may be invalid)
     */
    ImuData GetSpecificImuData(String parameter);
    
    /**
     * @brief Send joystick data to server via gRPC call
     * 
     * @param joystickData Joystick control data to send
     * @return GrpcResponse Response from server
     */
    GrpcResponse SendJoystickData(const JoystickData& joystickData);
    
    /**
     * @brief Start streaming IMU data from server
     * 
     * @param rate Streaming rate in Hz (default 10Hz)
     * @return GrpcResponse Response from server
     */
    GrpcResponse StartImuStreaming(int rate = 10);
    
    /**
     * @brief Check for and process streaming IMU data
     * 
     * @return ImuData Latest streaming data (valid=false if no new data)
     */
    ImuData GetStreamingImuData();
    
    /**
     * @brief Stop IMU data streaming
     * 
     * @return bool True if streaming was stopped successfully
     */
    bool StopImuStreaming();

private:
    /**
     * @brief Send a gRPC-like request to server
     * 
     * @param method RPC method name
     * @param params Parameters (optional)
     * @return String Raw response from server
     */
    String SendRequest(String method, String params = "");
    
    /**
     * @brief Parse JSON response into GrpcResponse structure
     * 
     * @param jsonResponse Raw JSON response
     * @return GrpcResponse Parsed response
     */
    GrpcResponse ParseResponse(String jsonResponse);
    
    /**
     * @brief Parse JSON response into ImuData structure
     * 
     * @param jsonResponse Raw JSON response
     * @return ImuData Parsed IMU data
     */
    ImuData ParseImuData(String jsonResponse);

    // Connection details
    IPAddress m_ServerIP;
    int m_ServerPort;
    WiFiClient m_Client;
    
    // Connection timeout (ms)
    static const int CONNECTION_TIMEOUT = 5000;
    static const int RESPONSE_TIMEOUT = 3000;
    
    // Streaming state
    bool m_IsReceivingStream;
    unsigned long m_LastStreamTime;
};

#endif // !GRPC_CLIENT_H