#include "include/config.h"
#include "include/led_manager.h"
#include "include/sensor_manager.h"
#include "include/movement_controller.h"
#include "include/command_processor.h"
#include "include/message_manager.h"

// Global instances of manager classes
LedManager ledManager;
SensorManager sensorManager;
MovementController* movementController;
CommandProcessor* commandProcessor;

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
  
  Serial.println("\n\nBCI-Controlled Test-bench Vehicle");
  
  // Initialize LED manager
  ledManager.init();
  
  // Initialize sensor manager
  sensorManager.init(ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);
  
  // Initialize movement controller
  movementController = new MovementController(&ledManager);
  movementController->init();
  
  // Initialize command processor
  commandProcessor = new CommandProcessor(movementController, &sensorManager, nullptr);
  
  // Print system information and instructions
  MessageManager::send("System ready - Connected via USB Serial");
  MessageManager::send("Left LED = movement/obstacles, Right LED = operational status");
  
  // Set initial watchdog time
  lastWatchdogTime = millis();
  
  // Print help info to the serial console
  MessageManager::send("\nAvailable commands:");
  commandProcessor->printHelpInfo();
}

void loop() {
  unsigned long currentMillis = millis();
  
  // Handle string input
  static const int MAX_BUFFER_SIZE = 64;
  
  // LED updates (important for user feedback)
  static unsigned long lastLedUpdate = 0;
  if (currentMillis - lastLedUpdate >= 20) {
    lastLedUpdate = currentMillis;
    ledManager.updateStatus(currentMillis, true); // Always show connected status
  }
  
  // Check for movement completion and avoidance maneuver updates
  movementController->checkTimedMovements(currentMillis);
  movementController->updateAvoidanceManeuver(currentMillis);
  
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
      MessageManager::sendF("Obstacle detected! %dcm", sensorManager.getValidDistance());
      movementController->performAvoidanceManeuver();
    }
  }
  
  // Process serial input - this is now our primary way to receive commands
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
    
    // Send periodic status update
    MessageManager::send("System running - ready for commands");
  }
}