/**
 * @file GrpcClient.cpp
 * @author Arunkumar Mourougappane (amouroug@buffalo.edu)
 * @brief Implementation of gRPC client for ESP32 rover control
 * @version 1.0.0
 * @date 2025-10-26
 *
 * Copyright (c) Arunkumar Mourougappane
 *
 */

#include "GrpcClient.h"

// gRPC-like message types (must match server)
#define MSG_LED_ON "TurnLedOn"
#define MSG_LED_OFF "TurnLedOff"
#define MSG_GET_ALL_IMU "GetAllImuData"
#define MSG_GET_SPECIFIC_IMU "GetSpecificImuData"
#define MSG_SEND_JOYSTICK "SendJoystickData"
#define MSG_STREAM_IMU "StreamImuData"

CGrpcClient::CGrpcClient(IPAddress serverIP, int serverPort)
    : m_ServerIP(serverIP), m_ServerPort(serverPort), m_IsReceivingStream(false), m_LastStreamTime(0)
{
}

bool CGrpcClient::Connect()
{
    log_i("Connecting to gRPC server at %s:%d", m_ServerIP.toString().c_str(), m_ServerPort);
    
    bool connected = m_Client.connect(m_ServerIP, m_ServerPort);
    
    if (connected)
    {
        log_i("Successfully connected to gRPC server");
    }
    else
    {
        log_e("Failed to connect to gRPC server");
    }
    
    return connected;
}

void CGrpcClient::Disconnect()
{
    if (m_Client.connected())
    {
        m_Client.stop();
        log_i("Disconnected from gRPC server");
    }
}

bool CGrpcClient::IsConnected()
{
    return m_Client.connected();
}

GrpcResponse CGrpcClient::TurnLedOn()
{
    String response = SendRequest(MSG_LED_ON);
    return ParseResponse(response);
}

GrpcResponse CGrpcClient::TurnLedOff()
{
    String response = SendRequest(MSG_LED_OFF);
    return ParseResponse(response);
}

ImuData CGrpcClient::GetAllImuData()
{
    String response = SendRequest(MSG_GET_ALL_IMU);
    return ParseImuData(response);
}

ImuData CGrpcClient::GetSpecificImuData(String parameter)
{
    String response = SendRequest(MSG_GET_SPECIFIC_IMU, parameter);
    return ParseImuData(response);
}

String CGrpcClient::SendRequest(String method, String params)
{
    if (!IsConnected())
    {
        log_e("Not connected to server");
        return "";
    }
    
    // Send request in format: METHOD:PARAMS
    String request = method;
    if (params.length() > 0)
    {
        request += ":" + params;
    }
    
    log_d("Sending request: %s", request.c_str());
    
    m_Client.println(request);
    m_Client.flush();
    
    // Wait for response
    unsigned long startTime = millis();
    while (!m_Client.available() && (millis() - startTime) < RESPONSE_TIMEOUT)
    {
        delay(1);
    }
    
    if (!m_Client.available())
    {
        log_e("Response timeout");
        return "";
    }
    
    // Read response
    String response = m_Client.readStringUntil('\n');
    response.trim();
    
    log_d("Received response: %s", response.c_str());
    
    // Parse LENGTH:DATA format
    int colonIndex = response.indexOf(':');
    if (colonIndex > 0)
    {
        int expectedLength = response.substring(0, colonIndex).toInt();
        String data = response.substring(colonIndex + 1);
        
        if (data.length() == expectedLength)
        {
            return data;
        }
        else
        {
            log_e("Response length mismatch. Expected: %d, Got: %d", expectedLength, data.length());
        }
    }
    
    return response; // Return as-is if parsing fails
}

GrpcResponse CGrpcClient::ParseResponse(String jsonResponse)
{
    GrpcResponse response;
    response.success = false;
    response.timestamp = 0;
    
    if (jsonResponse.length() == 0)
    {
        response.error = "Empty response";
        return response;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error)
    {
        response.error = "JSON parsing failed: " + String(error.c_str());
        log_e("JSON parsing failed: %s", error.c_str());
        return response;
    }
    
    response.success = doc["success"] | false;
    // Try message first, then data, then empty string
    if (doc["message"].is<const char*>())
        response.data = doc["message"].as<const char*>();
    else if (doc["data"].is<const char*>())
        response.data = doc["data"].as<const char*>();
    else
        response.data = "";
    response.error = doc["error"] | "";
    response.timestamp = doc["timestamp"] | 0;
    
    return response;
}

ImuData CGrpcClient::ParseImuData(String jsonResponse)
{
    ImuData imuData;
    memset(&imuData, 0, sizeof(ImuData));
    imuData.valid = false;
    
    if (jsonResponse.length() == 0)
    {
        log_e("Empty IMU data response");
        return imuData;
    }
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonResponse);
    
    if (error)
    {
        log_e("IMU JSON parsing failed: %s", error.c_str());
        return imuData;
    }
    
    // Check if request was successful
    if (!(doc["success"] | false))
    {
        String errorMsg = doc["error"] | "Unknown error";
        log_e("IMU data request failed: %s", errorMsg.c_str());
        return imuData;
    }
    
    // Parse IMU data fields
    imuData.acc_x = doc["acc_x"] | 0.0f;
    imuData.acc_y = doc["acc_y"] | 0.0f;
    imuData.acc_z = doc["acc_z"] | 0.0f;
    imuData.gyro_x = doc["gyro_x"] | 0.0f;
    imuData.gyro_y = doc["gyro_y"] | 0.0f;
    imuData.gyro_z = doc["gyro_z"] | 0.0f;
    imuData.temperature = doc["temperature"] | 0.0f;
    imuData.timestamp = doc["timestamp"] | 0;
    imuData.valid = true;
    
    return imuData;
}

GrpcResponse CGrpcClient::SendJoystickData(const JoystickData& joystickData)
{
    // Create JSON payload for joystick data
    JsonDocument doc;
    doc["left_x"] = joystickData.left_x;
    doc["left_y"] = joystickData.left_y;
    doc["right_x"] = joystickData.right_x;
    doc["right_y"] = joystickData.right_y;
    doc["left_button"] = joystickData.left_button;
    doc["right_button"] = joystickData.right_button;
    doc["timestamp"] = joystickData.timestamp;
    
    String jsonPayload;
    serializeJson(doc, jsonPayload);
    
    log_d("Sending joystick data: %s", jsonPayload.c_str());
    
    String response = SendRequest(MSG_SEND_JOYSTICK, jsonPayload);
    return ParseResponse(response);
}

GrpcResponse CGrpcClient::StartImuStreaming(int rate)
{
    if (!IsConnected())
    {
        GrpcResponse response;
        response.success = false;
        response.error = "Not connected to server";
        return response;
    }
    
    // Create streaming parameters
    JsonDocument params;
    params["rate"] = rate;
    
    String paramsString;
    serializeJson(params, paramsString);
    
    log_i("Starting IMU streaming at %d Hz", rate);
    
    String response = SendRequest(MSG_STREAM_IMU, paramsString);
    GrpcResponse grpcResponse = ParseResponse(response);
    
    if (grpcResponse.success)
    {
        m_IsReceivingStream = true;
        m_LastStreamTime = millis();
        log_i("IMU streaming started successfully");
    }
    else
    {
        log_e("Failed to start IMU streaming: %s", grpcResponse.error.c_str());
    }
    
    return grpcResponse;
}

ImuData CGrpcClient::GetStreamingImuData()
{
    ImuData imuData;
    memset(&imuData, 0, sizeof(ImuData));
    imuData.valid = false;
    
    if (!IsConnected() || !m_IsReceivingStream)
    {
        return imuData;
    }
    
    // Check for streaming data
    if (m_Client.available())
    {
        String response = m_Client.readStringUntil('\n');
        response.trim();
        
        log_d("Received stream data: %s", response.c_str());
        
        // Check if it's streaming data
        if (response.startsWith("STREAM:"))
        {
            // Parse STREAM:LENGTH:DATA format
            int firstColon = response.indexOf(':');
            int secondColon = response.indexOf(':', firstColon + 1);
            
            if (firstColon > 0 && secondColon > firstColon)
            {
                int expectedLength = response.substring(firstColon + 1, secondColon).toInt();
                String data = response.substring(secondColon + 1);
                
                if (data.length() == expectedLength)
                {
                    imuData = ParseImuData(data);
                    m_LastStreamTime = millis();
                }
                else
                {
                    log_e("Stream data length mismatch. Expected: %d, Got: %d", expectedLength, data.length());
                }
            }
        }
        else if (response.startsWith("STREAM_END:"))
        {
            log_i("Stream ended by server");
            m_IsReceivingStream = false;
        }
    }
    
    return imuData;
}

bool CGrpcClient::StopImuStreaming()
{
    if (m_IsReceivingStream)
    {
        m_IsReceivingStream = false;
        log_i("IMU streaming stopped");
        return true;
    }
    return false;
}