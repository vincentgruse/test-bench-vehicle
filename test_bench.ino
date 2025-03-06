#include "include/config.h"
#include "include/led_manager.h"
#include "include/bt_manager.h"
#include "include/sensor_manager.h"
#include "include/movement_controller.h"
#include "include/command_processor.h"

// Check if Bluetooth is available
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` and enable it
#endif

// Check Serial Port Profile
#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Port Profile for Bluetooth is not available or not enabled. It is only available for the ESP32 chip.
#endif

// Global instances of manager classes
LedManager ledManager;
BtManager* btManager;
SensorManager sensorManager;
MovementController* movementController;
CommandProcessor* commandProcessor;

// Make it available for movement controller messages
BtManager* btManagerInstance;

// Input buffer for commands
String inputString = "";

// Watchdog timer for monitoring system health
unsigned long lastWatchdogTime = 0;
const unsigned long WATCHDOG_INTERVAL = 30000; // Check every 30 seconds

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Wait for serial to initialize
  delay(1000);
  
  Serial.println("\n\nTest-bench Car Control System");
  
  // Initialize LED manager
  ledManager.init();
  
  // Initialize Bluetooth manager
  btManager = new BtManager();
  btManagerInstance = btManager; // Set the global instance reference
  btManager->init(BT_DEVICE_NAME);
  
  // Initialize sensor manager
  sensorManager.init(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);
  
  // Initialize movement controller
  movementController = new MovementController(&ledManager);
  movementController->init();
  
  // Initialize command processor
  commandProcessor = new CommandProcessor(movementController, &sensorManager, btManager);
  
  // Print system information and instructions
  Serial.println("System ready - Pair with device: " + String(BT_DEVICE_NAME));
  Serial.println("LEDs: Right = connection, Left = movement/obstacles");
  
  // Set initial watchdog time
  lastWatchdogTime = millis();
  
  // Print help info to the serial console
  Serial.println("\nAvailable commands:");
  commandProcessor->printHelpInfo();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Handle string input
  static const int MAX_BUFFER_SIZE = 64;
  
  // Update Bluetooth connection status
  btManager->updateConnectionStatus(currentMillis);
  
  // LED updates (important for user feedback)
  static unsigned long lastLedUpdate = 0;
  if (currentMillis - lastLedUpdate >= 20) {
    lastLedUpdate = currentMillis;
    ledManager.updateStatus(currentMillis, btManager->isDeviceConnected());
  }
  
  // Check for movement completion and avoidance maneuver updates
  movementController->checkTimedMovements(currentMillis);
  movementController->updateAvoidanceManeuver(currentMillis);
  
  // Process Bluetooth input
  btManager->processBtInput(currentMillis, inputString);
  
  // Check buffer size and truncate if necessary
  if (inputString.length() > MAX_BUFFER_SIZE) {
    inputString = inputString.substring(inputString.length() - MAX_BUFFER_SIZE);
  }
  
  // Check for obstacles when necessary
  static unsigned long lastObstacleCheck = 0;
  if (currentMillis - lastObstacleCheck >= OBSTACLE_CHECK_INTERVAL) {
    lastObstacleCheck = currentMillis;
    
    if (sensorManager.checkForObstacles(currentMillis)) {
      // Obstacle detected, stop and turn
      movementController->stop();
      movementController->cancelTimedMovement();
      btManager->sendMessageF("Obstacle detected! %dcm", sensorManager.getValidDistance());
      movementController->performAvoidanceManeuver();
    }
  }
  
  // Process serial input
  if (Serial.available() > 0) {
    commandProcessor->processSerialInput(inputString);
  }
  
  // Process any complete commands
  int newlineIndex;
  while ((newlineIndex = inputString.indexOf('\n')) >= 0 || 
         (newlineIndex = inputString.indexOf('\r')) >= 0) {
    
    // Extract and process the command
    String cmd = inputString.substring(0, newlineIndex);
    cmd.trim();
    
    if (cmd.length() > 0) {
      commandProcessor->processCommand(cmd);
    }
    
    // Remove the processed command from the buffer
    inputString = inputString.substring(newlineIndex + 1);
  }
  
  // Lower priority maintenance tasks
  if (currentMillis - lastWatchdogTime >= WATCHDOG_INTERVAL) {
    lastWatchdogTime = currentMillis;
    
    // Send ping if connected
    static unsigned long lastPingTime = 0;
    if (btManager->isDeviceConnected() && (currentMillis - lastPingTime >= 60000)) {
      lastPingTime = currentMillis;
      btManager->sendPing();
    }
  }
}