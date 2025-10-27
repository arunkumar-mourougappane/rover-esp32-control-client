# ESP32 Rover Controller Client

A gRPC-based remote controller client using Adafruit Feather ESP32-S3 TFT Reverse hardware, designed to control a rover with dual analog joysticks and receive real-time IMU data streaming.

## 🎮 Features

- **gRPC-like Protocol**: TCP-based communication protocol for reliable rover control
- **Dual Joystick Control**: Support for two analog joysticks with button inputs
- **Real-time IMU Streaming**: Continuous accelerometer and gyroscope data reception
- **LED Control**: Remote LED on/off control with status feedback
- **WiFi Connectivity**: Connects to rover's access point for communication
- **Optimized Performance**: Efficient memory usage (17.4% RAM, 52.1% Flash)

## 🛠 Hardware Requirements

### ESP32-S3 Board

- **Adafruit Feather ESP32-S3 TFT Reverse** (primary target)
- ESP32-S3 with WiFi capability
- Minimum 4MB Flash, 320KB RAM

### Peripheral Components

- **2x Analog Joysticks**: Connected to ADC pins A0-A3
- **2x Push Buttons**: Connected to GPIO5, GPIO6 (joystick buttons)
- **Power Supply**: USB-C or battery power

## 📡 Communication Protocol

The client communicates with the rover server using a gRPC-like TCP protocol on port **50051**.

### Message Types

- `MSG_SET_LED`: Control rover's LED on/off state
- `MSG_GET_IMU`: Request IMU data (single reading)
- `MSG_JOYSTICK_DATA`: Send joystick control data
- `MSG_STREAM_IMU`: Start/stop IMU data streaming

### Streaming Protocol

- **Format**: `STREAM:length:data` for continuous data
- **End Marker**: `STREAM_END:length:data` for stream termination
- **Configurable Rate**: 1-50Hz streaming frequency

## 🚀 Getting Started

### Prerequisites

- PlatformIO IDE or CLI
- ESP32 development environment
- Access to rover's WiFi network (`MOONBASE-II`)

### Build and Upload

```bash
# Clone the repository
git clone https://github.com/arunkumar-mourougappane/rover-esp32-control-client.git
cd rover-esp32-control-client

# Build the project
platformio run

# Upload to ESP32-S3
platformio run --target upload

# Monitor serial output
platformio device monitor
```

### Configuration

Update WiFi credentials in `src/main.cpp`:

```cpp
const String ROVER_AP_SSID = String("MOONBASE-II");
const String ROVER_AP_PASS_PHRASE = String("Trypt1c0n$");
```

## 📋 Pin Configuration

| Component        | Pin   | GPIO  | Description               |
| ---------------- | ----- | ----- | ------------------------- |
| Left Joystick X  | A0    | GPIO1 | Left analog stick X-axis  |
| Left Joystick Y  | A1    | GPIO2 | Left analog stick Y-axis  |
| Right Joystick X | A2    | GPIO3 | Right analog stick X-axis |
| Right Joystick Y | A3    | GPIO4 | Right analog stick Y-axis |
| Left Button      | GPIO5 | GPIO5 | Left joystick button      |
| Right Button     | GPIO6 | GPIO6 | Right joystick button     |

## 🎯 Usage

### Joystick Control

The controller reads dual analog joysticks at 50Hz and sends control data to the rover:

- **Left Stick**: Typically used for movement (forward/backward, left/right)
- **Right Stick**: Typically used for rotation or camera control
- **Buttons**: Additional control inputs

### IMU Data Streaming

The client receives continuous IMU data from the rover:

- **Accelerometer**: 3-axis acceleration data (m/s²)
- **Gyroscope**: 3-axis angular velocity data (rad/s)
- **Temperature**: Sensor temperature (°C)
- **Timestamp**: Millisecond timestamp for data correlation

### LED Control

Simple LED on/off control for rover status indication or testing connectivity.

## 📚 Library Structure

### Core Libraries

- **GrpcClient**: Main communication library
- **ArduinoJson v7.2.1**: JSON serialization/deserialization
- **WiFi**: ESP32 WiFi connectivity

### Key Components

- `GrpcClient.h/cpp`: gRPC-like protocol implementation
- `JoystickData.h`: Joystick data structure definitions
- `SensorData.h`: IMU and sensor data structures
- `proto/rover_service.proto`: Protocol service definitions

## 🔧 Development

### Building Custom Features

The modular architecture allows easy extension:

1. Add new message types in `proto/rover_service.proto`
2. Implement handlers in `GrpcClient` library
3. Update main loop for new functionality

### Debugging

- Serial output at 115200 baud
- Debug logging with `log_i()`, `log_e()` macros
- Network connectivity status monitoring

## 📊 Performance Metrics

- **RAM Usage**: 17.4% (57,012 bytes / 327,680 bytes)
- **Flash Usage**: 52.1% (751,501 bytes / 1,441,792 bytes)
- **Communication Rate**: Up to 50Hz joystick data, configurable IMU streaming
- **WiFi Range**: Typical ESP32 WiFi range (~100m line of sight)

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on hardware
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.
