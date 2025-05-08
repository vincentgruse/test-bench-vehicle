# Test-Bench Car Control System

A robust, Bluetooth-enabled control system for an ESP32-based robotic test vehicle with obstacle avoidance capabilities.

## Table of Contents

- [Overview](#overview)
- [Hardware Requirements](#hardware-requirements)
  - [Wiring](#wiring)
- [Software Dependencies](#software-dependencies)
  - [Installing ESP32 Board Support](#installing-esp32-board-support)
  - [Setting Up ESP32 Board](#setting-up-esp32-board)
  - [BluetoothSerial Library](#bluetoothserial-library)
  - [Custom Libraries](#custom-libraries)
  - [USB Drivers (Windows Only)](#usb-drivers-windows-only)
- [Installation](#installation)
  - [Project Structure](#project-structure)
- [Configuration](#configuration)
- [Command Reference](#command-reference)
- [LED Status Indicators](#led-status-indicators)
- [Bluetooth Connectivity](#bluetooth-connectivity)
  - [Connecting with PuTTY](#connecting-with-putty)
- [Obstacle Avoidance](#obstacle-avoidance)
- [Sensor Reliability Features](#sensor-reliability-features)
- [System Architecture](#system-architecture)
- [Troubleshooting](#troubleshooting)
- [Performance Considerations](#performance-considerations)
- [Development Notes](#development-notes)
- [Future Improvements](#future-improvements)
- [License](#license)

## Overview

This project implements a complete control system for a small robotic vehicle, featuring:

- Bluetooth control via a simple command interface
- Configurable speed control
- Obstacle detection and avoidance using ultrasonic sensor
- LED status indicators for system feedback
- Timed movements with non-blocking execution
- Robust command parsing with shortcuts
- Multiple communication interfaces (Bluetooth and Serial)

## Hardware Requirements

- ESP32 development board
- Robotic vehicle chassis with motors
- Motor driver compatible with the `vehicle` library
- HC-SR04 ultrasonic sensor (or compatible)
- 2 LEDs (for status indicators)
- Power supply appropriate for your motors

### Wiring

| Component | Pin |
|-----------|-----|
| Left LED | GPIO 12 |
| Right LED | GPIO 2 |
| Ultrasonic Trigger | GPIO 13 |
| Ultrasonic Echo | GPIO 14 |
| Motor control | Managed by vehicle library |

## Software Dependencies

- Arduino IDE (1.8.x or 2.x)
- ESP32 board support package
- BluetoothSerial library (included with ESP32 board support)
- Custom `vehicle` library (included in project)
- Custom `ultrasonic` library (included in project)

### Installing ESP32 Board Support

1. Open Arduino IDE
2. Go to **File > Preferences**
3. In the "Additional Boards Manager URLs" field, add:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click "OK"
5. Go to **Tools > Board > Boards Manager**
6. Search for "ESP32"
7. Install "ESP32 by Espressif Systems" (select the latest version)
8. Wait for the installation to complete
9. Restart Arduino IDE

### Setting Up ESP32 Board

1. Go to **Tools > Board > ESP32 Arduino**
2. Select "ESP32 Dev Module" from the list
3. Configure the following settings (under the Tools menu):
   - Upload Speed: 921600
   - CPU Frequency: 240MHz
   - Flash Frequency: 80MHz
   - Flash Mode: QIO
   - Flash Size: 4MB
   - Partition Scheme: Default
   - Core Debug Level: None
   - PSRAM: Disabled

### BluetoothSerial Library

The BluetoothSerial library is automatically included with the ESP32 board package, so no additional installation is required. You can include it in your code with:

```cpp
#include "BluetoothSerial.h"
```

### Custom Libraries

The project uses two custom libraries that are included in the project:

1. **vehicle**: Handles motor control with several movement modes:
   - `Forward`: Move forward
   - `Backward`: Move backward
   - `Clockwise`: Turn clockwise (right)
   - `Contrarotate`: Turn counterclockwise (left)
   - `Stop`: Stop all motion

2. **ultrasonic**: Manages the HC-SR04 ultrasonic distance sensor.

### USB Drivers (Windows Only)

If your computer doesn't recognize the ESP32 board:

1. Download the CP210x USB driver from [Silicon Labs website](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
2. Install the driver appropriate for your operating system
3. Reconnect your ESP32 board
4. Check Device Manager to verify it appears as a COM port

## Installation

1. Install Arduino IDE and ESP32 board support
2. Clone or download this repository
3. **Important Library Setup**:
   - Ensure the `vehicle` and `ultrasonic` library files are in the correct locations in the project structure
   - Verify that the case of filenames matches exactly with your #include statements
4. Open `test_bench.ino` in Arduino IDE
5. Select your ESP32 board from Tools > Board menu
6. Connect your ESP32 via USB
7. Click Upload

### Project Structure

The project is organized as follows:

```
test_bench/
│
├── test_bench.ino              # Main Arduino sketch
│
├── include/                    # Header files
│   ├── config.h                # Configuration constants
│   ├── bt_manager.h            # Bluetooth communication
│   ├── command_processor.h     # Command parsing and handling
│   ├── led_manager.h           # LED status indicators
│   ├── movement_controller.h   # Vehicle movement control
│   ├── sensor_manager.h        # Ultrasonic sensor management
│   ├── serial_manager.h        # Serial communication
│   └── message_manager.h       # Abstract message handling
│
└── src/                        # Implementation files
    ├── bt_manager.cpp
    ├── command_processor.cpp
    ├── led_manager.cpp
    ├── movement_controller.cpp
    ├── sensor_manager.cpp
    │
    └── lib/                    # External libraries
        ├── vehicle/            # Vehicle motor control library
        │   ├── vehicle.cpp
        │   └── vehicle.h
        │
        └── ultrasonic/         # Ultrasonic sensor library
            ├── ultrasonic.cpp
            └── ultrasonic.h
```

## Configuration

Key configuration options can be found in `config.h`:

### Bluetooth and Connection Settings
- `BT_DEVICE_NAME`: Name of the Bluetooth device (default: "test-bench")
- `BT_TIMEOUT`: Bluetooth inactivity timeout (60000 ms)
- `CONNECTION_BLINK_INTERVAL`: Blink interval when not connected (500 ms)

### Movement Settings
- `DEFAULT_SPEED`: Default movement speed (150)
- `MIN_SPEED`: Minimum allowed speed (50)
- `MAX_SPEED`: Maximum allowed speed (255)
- `TURN_SPEED`: Speed used for turning (180)

### LED Indicators
- `LEFT_LED`: GPIO pin for left LED (12)
- `RIGHT_LED`: GPIO pin for right LED (2)
- `MOVEMENT_BLINK_INTERVAL`: Standard movement blink interval (300 ms)
- `FAST_BLINK_INTERVAL`: Fast blink interval for turning (150 ms)
- `OBSTACLE_BLINK_INTERVAL`: Very fast blink for obstacle detection (100 ms)

### Sensor Settings
- `ULTRASONIC_TRIG_PIN`: Trigger pin for ultrasonic sensor (13)
- `ULTRASONIC_ECHO_PIN`: Echo pin for ultrasonic sensor (14)
- `OBSTACLE_CHECK_INTERVAL`: How often to check for obstacles (200 ms)
- `MIN_VALID_DISTANCE`: Minimum valid distance reading (2 cm)
- `MAX_VALID_DISTANCE`: Maximum valid distance reading (400 cm)
- `MAX_READING_ATTEMPTS`: Number of sensor reading attempts (3)
- `FALLBACK_DISTANCE`: Default value when readings fail (1000 cm)
- `OBSTACLE_DETECTION_DISTANCE`: Distance threshold for obstacle detection (25 cm)

## Command Reference

The following commands can be sent via Bluetooth or Serial:

### Speed Control

- `speed [value]`: Set global speed (50-255)

### Movement Commands

- `forward` or `f`: Move forward at current speed
- `forward [seconds]` or `f [seconds]`: Move forward for specified seconds at current speed
- `forward [speed] [seconds]` or `f [speed] [seconds]`: Move forward at specific speed for specified seconds
- `backward` or `b`: Move backward at current speed
- `backward [seconds]` or `b [seconds]`: Move backward for specified seconds at current speed
- `backward [speed] [seconds]` or `b [speed] [seconds]`: Move backward at specific speed for specified seconds
- `stop` or `s`: Stop movement
- `turn X`: Turn by X degrees (positive for right, negative for left)

### Sensor Commands

- `distance`: Report current distance from ultrasonic sensor
- `avoid on/off`: Enable/disable obstacle avoidance
- `debug on/off`: Enable/disable sensor debugging information

### Other Commands

- `help`: Show help information
- `ping`: Simple connectivity test
- `status`: Show current system status (connection, speed, etc.)

## LED Status Indicators

### Right LED
- **Solid On**: Bluetooth connected
- **Blinking**: Not connected (waiting for connection)

### Left LED
- **Off**: Vehicle idle
- **Solid On**: Moving forward
- **Slow Blinking**: Moving backward
- **Fast Blinking**: Turning
- **Double Flash**: Obstacle detected, performing avoidance maneuver
- **Alternating with Right LED**: Error condition

## Bluetooth Connectivity

1. Power on the vehicle
2. On your Bluetooth device, scan for a device named "test-bench" (or your configured name)
3. Pair with the device (no PIN required)
4. Once connected, send commands via a serial terminal app
5. Each command should end with a newline character

### Connecting with PuTTY

PuTTY is a free terminal emulator that can be used to send commands to your vehicle:

1. **Install PuTTY**: Download and install from [https://www.putty.org/](https://www.putty.org/)

2. **Find COM port**:
   - After pairing with the ESP32, open Device Manager on Windows
   - Expand "Ports (COM & LPT)"
   - Look for "Standard Serial over Bluetooth link (COM#)" - note the COM# (e.g., COM3)

3. **Configure PuTTY**:
   - Connection Type: Serial
   - Serial line: COM# (from the previous step)
   - Speed: 115200
   - Data bits: 8
   - Stop bits: 1
   - Parity: None
   - Flow control: None

4. **Additional settings**:
   - In the Category panel, go to Terminal
   - Set "Local echo" to "Force on"
   - Set "Local line editing" to "Force on"
   - Under "Line discipline options," set "Implicit CR in every LF" to "on"

5. **Connect**:
   - Click "Open" to start the session
   - The terminal window will appear
   - Type commands and press Enter to send them to the vehicle

6. **Save settings** (optional):
   - Return to the configuration screen
   - Enter a name in the "Saved Sessions" field
   - Click "Save" to save this configuration for future use

## Obstacle Avoidance

Obstacle avoidance is enabled by default. The system uses multiple ultrasonic sensor readings with filtering to ensure reliable distance measurements.

When an obstacle is detected within the configured detection distance (default 25cm):

1. The vehicle immediately stops any current movement
2. The left LED changes to the obstacle pattern (double-flash)
3. The avoidance maneuver state machine activates:
   - **AVOID_BACKING**: Vehicle backs up for 500ms
   - **AVOID_TURNING**: Vehicle turns left for 1000ms
   - **AVOID_IDLE**: Returns to normal operation

This non-blocking implementation ensures the vehicle remains responsive during the avoidance maneuver.

Use `avoid off` to disable this feature and `avoid on` to re-enable it.

## Sensor Reliability Features

The SensorManager implements several reliability mechanisms:

1. **Multiple readings**: Takes multiple samples for each distance check
2. **Median filtering**: Uses median of readings to reject outliers
3. **Range validation**: Ignores readings outside valid distance range
4. **Failure detection**: Tracks consecutive failed readings
5. **Fallback values**: Returns safe values when sensor fails
6. **Adaptive checking**: Adjusts sensor check frequency based on detected distance

Debug mode (`debug on`) provides detailed information about sensor readings for troubleshooting.

## System Architecture

The system is organized into the following modules:

1. **CommandProcessor**: Handles command parsing and execution
   - Parses incoming commands in a case-insensitive manner
   - Supports shorthand commands (`f` for forward, `b` for backward, etc.)
   - Routes commands to appropriate modules

2. **BtManager**: Manages Bluetooth communication
   - Handles device discovery and connection
   - Processes incoming commands
   - Provides formatted message sending
   - Monitors connection state with timeout detection

3. **SerialManager**: Manages USB Serial communication
   - Provides alternative control interface via USB
   - Mirrors the BtManager interface for consistency

4. **MessageManager**: Abstract message handling
   - Provides a common interface for all message types
   - Simplifies switching between communication methods

5. **MovementController**: Controls vehicle movement
   - Manages motor control via the vehicle library
   - Supports timed movements with non-blocking execution
   - Implements speed control with minimum/maximum constraints
   - Handles the obstacle avoidance state machine

6. **SensorManager**: Manages the ultrasonic sensor
   - Provides filtered, reliable distance readings
   - Implements obstacle detection logic
   - Supports debug mode for troubleshooting
   - Handles sensor failure gracefully
   - Uses adaptive timing for efficient obstacle detection

7. **LedManager**: Controls the status LEDs
   - Provides visual feedback on system state
   - Implements different blink patterns for each state
   - Manages connection status indication

The main loop in `test_bench.ino` orchestrates these modules with priority-based task scheduling to ensure smooth operation.

## Troubleshooting

### No Bluetooth Connection
1. Check that the right LED is blinking (indicating it's waiting for connection)
2. Ensure your device supports Bluetooth Serial Port Profile (SPP)
3. Try deleting the device from your Bluetooth settings and reconnecting
4. Verify the ESP32 board has Bluetooth enabled in the Arduino IDE settings
5. Restart the vehicle

### Unresponsive Movement
1. Check battery levels
2. Verify motor connections
3. Send `status` command to check current settings
4. Ensure speed is set appropriately (`speed 150`)
5. Check serial console for error messages

### Erratic Sensor Readings
1. Check sensor wiring and connections
2. Enable debug mode with `debug on`
3. Check distance readings with `distance`
4. Ensure the sensor is not obstructed
5. Verify the sensor is mounted securely

### Commands Not Recognized
1. Ensure commands are lowercase
2. Check that each command ends with a newline character
3. Try the `ping` command to verify communication
4. Inspect the input buffer for unexpected characters
5. Check for command typos (use `help` to see available commands)

## Performance Considerations

- The system uses non-blocking operations for smooth performance
- String handling is optimized to reduce memory fragmentation
- The main loop prioritizes critical tasks for better responsiveness
- Obstacle detection is optimized to reduce unnecessary processing
- Sensor readings use filtering to improve reliability
- Adaptive timing reduces sensor polling when not needed

## Development Notes

For developers extending this codebase:

- Use the `sendMessageF` method for efficient string formatting
- Follow the state machine pattern for new non-blocking operations
- Respect the main loop priorities when adding new features
- Use the command parser for new commands
- Implement new LED patterns using the existing framework
- Leverage the abstracted communication architecture to add new interfaces

## Future Improvements

Potential enhancements for future development:

- Add remote diagnostics and telemetry
- Implement PID control for more precise movements
- Add a web interface for control and monitoring
- Support for additional sensors (IMU, encoders)
- Path planning and mapping capabilities
- Battery voltage monitoring
- Configuration persistence in EEPROM
- Custom mobile app for control

## License

This project is available under the MIT license.