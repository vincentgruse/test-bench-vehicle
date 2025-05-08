#include "include/config.h"
#include "include/led_manager.h"
#include "include/sensor_manager.h"
#include "include/movement_controller.h"
#include "include/command_processor.h"
#include "include/message_manager.h"
#include "include/bt_manager.h"
#include "include/serial_manager.h"

// Global instances of manager classes
LedManager ledManager;
SerialManager serialManager;
BtManager btManager;
SensorManager sensorManager;
MovementController* movementController;
CommandProcessor* commandProcessor;

// Input buffers for commands
String serialInputString = "";
String btInputString = "";

// Connection state tracking
bool usbConnected = false;
bool btActive = false;

// Watchdog timer for monitoring system health
unsigned long lastWatchdogTime = 0;
const unsigned long WATCHDOG_INTERVAL = 30000; // Check every 30 seconds

// Connection check interval
const unsigned long CONNECTION_CHECK_INTERVAL = 1000; // Check connection every second
unsigned long lastConnectionCheck = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Wait for serial to initialize
  delay(1000);
  
  Serial.println("\n\nBCI-Controlled Test-bench Vehicle");
  
  // Initialize LED manager
  ledManager.init();
  
  // Initialize sensor manager
  sensorManager.init(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);
  
  // Initialize movement controller
  movementController = new MovementController(&ledManager);
  movementController->init();
  
  // Check initial USB connection status
  usbConnected = serialManager.checkConnection();
  
  // Only initialize BT if USB is not connected
  if (!usbConnected) {
    btManager.init(BT_DEVICE_NAME);
    btActive = true;
    Serial.println("USB not connected - starting in Bluetooth mode");
  } else {
    Serial.println("USB connected - starting in USB Serial mode");
  }
  
  // Initialize command processor with appropriate BT manager
  commandProcessor = new CommandProcessor(movementController, &sensorManager, btActive ? &btManager : nullptr);
  
  // Print system information and instructions
  if (usbConnected) {
    MessageManager::send("System ready - Connected via USB Serial");
  } else {
    MessageManager::send("System ready - Waiting for Bluetooth connection");
  }
  MessageManager::send("Left LED = movement/obstacles, Right LED = operational status");
  
  // Set initial watchdog time
  lastWatchdogTime = millis();
  lastConnectionCheck = millis();
  
  // Print help info to the serial console
  MessageManager::send("\nAvailable commands:");
  commandProcessor->printHelpInfo();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Check USB connection status periodically
  if (currentMillis - lastConnectionCheck >= CONNECTION_CHECK_INTERVAL) {
    lastConnectionCheck = currentMillis;
    
    // Update USB connection status
    bool currentUsbStatus = serialManager.checkConnection();
    
    // Handle connection state changes
    if (currentUsbStatus != usbConnected) {
      usbConnected = currentUsbStatus;
      
      if (usbConnected) {
        // USB just connected
        Serial.println("USB connected, switching to serial control");
        
        // If BT is active, disable it
        if (btActive) {
          btManager.end();
          btActive = false;
          
          // Update command processor to use no BT
          delete commandProcessor; // Delete old processor first
          commandProcessor = new CommandProcessor(movementController, &sensorManager, nullptr);
        }
      } else {
        // USB just disconnected
        Serial.println("USB disconnected, enabling Bluetooth control");
        
        // Enable Bluetooth
        if (!btActive) {
          btManager.init(BT_DEVICE_NAME);
          btActive = true;
          
          // Update command processor to use BT
          delete commandProcessor; // Delete old processor first
          commandProcessor = new CommandProcessor(movementController, &sensorManager, &btManager);
        }
      }
    }
  }
  
  // LED updates
  static unsigned long lastLedUpdate = 0;
  if (currentMillis - lastLedUpdate >= 20) {
    lastLedUpdate = currentMillis;
    
    // Show connected status based on actual connections
    bool activeConnection = usbConnected || (btActive && btManager.isDeviceConnected());
    ledManager.updateStatus(currentMillis, activeConnection);
  }
  
  // Check for movement completion and avoidance maneuver updates
  movementController->checkTimedMovements(currentMillis);
  movementController->updateAvoidanceManeuver(currentMillis);
  
  // Handle string input
  static const int MAX_BUFFER_SIZE = 64;
  
  // Check buffer sizes and truncate if necessary
  if (serialInputString.length() > MAX_BUFFER_SIZE) {
    serialInputString = serialInputString.substring(serialInputString.length() - MAX_BUFFER_SIZE);
  }
  
  if (btInputString.length() > MAX_BUFFER_SIZE) {
    btInputString = btInputString.substring(btInputString.length() - MAX_BUFFER_SIZE);
  }
  
  // Check for obstacles when necessary
  static unsigned long lastObstacleCheck = 0;
  if (currentMillis - lastObstacleCheck >= OBSTACLE_CHECK_INTERVAL) {
    lastObstacleCheck = currentMillis;
    
    if (sensorManager.checkForObstacles(currentMillis)) {
      // Obstacle detected, stop and turn
      movementController->stop();
      movementController->cancelTimedMovement();
      MessageManager::sendF("Obstacle detected! %dcm", sensorManager.getValidDistance());
      movementController->performAvoidanceManeuver();
    }
  }
  
  // Process Bluetooth input if active and USB not connected
  if (btActive && !usbConnected) {
    // Update BT connection status
    btManager.updateConnectionStatus(currentMillis);
    
    // Process Bluetooth input if available
    btManager.processBtInput(currentMillis, btInputString);
    
    // Process any complete BT commands
    int btNewlineIndex;
    while ((btNewlineIndex = btInputString.indexOf('\n')) >= 0 || 
           (btNewlineIndex = btInputString.indexOf('\r')) >= 0) {
      
      // Extract and process the command
      String btCmd = btInputString.substring(0, btNewlineIndex);
      btCmd.trim();
      
      if (btCmd.length() > 0) {
        commandProcessor->processCommand(btCmd);
      }
      
      // Remove the processed command from the buffer
      btInputString = btInputString.substring(btNewlineIndex + 1);
    }
  }
  
  // Process serial input only if USB is connected
  if (usbConnected && Serial.available() > 0) {
    while (Serial.available() > 0) {
      char inChar = (char)Serial.read();
      serialInputString += inChar;
      
      // Process immediately if it's a newline
      if (inChar == '\n' || inChar == '\r') {
        if (serialInputString.length() > 1) { // Non-empty command
          String cmd = serialInputString;
          cmd.trim();
          commandProcessor->processCommand(cmd);
          serialInputString = "";
        }
      }
    }
  }
  
  // Lower priority maintenance tasks
  if (currentMillis - lastWatchdogTime >= WATCHDOG_INTERVAL) {
    lastWatchdogTime = currentMillis;
    
    // Send periodic status update based on connection mode
    if (usbConnected) {
      MessageManager::send("System running - USB mode active");
    } else if (btActive && btManager.isDeviceConnected()) {
      MessageManager::send("System running - Bluetooth connected");
    } else if (btActive) {
      MessageManager::send("System running - Waiting for Bluetooth connection");
    }
  }
}