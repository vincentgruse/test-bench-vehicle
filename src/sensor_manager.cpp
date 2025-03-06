#include "../include/sensor_manager.h"
#include "../include/bt_manager.h"

// External reference to the BtManager for sending messages
extern BtManager* btManagerInstance;

SensorManager::SensorManager() {
  sensor = new ultrasonic();
  debugEnabled = false;
  lastObstacleCheck = 0;
  avoidanceEnabled = true;
  consecutiveFailedReadings = 0;
  lastValidDistance = 0;
}

SensorManager::~SensorManager() {
  // Free memory on destruction
  if (sensor != nullptr) {
    delete sensor;
    sensor = nullptr;
  }
}

void SensorManager::init(int trigPin, int echoPin) {
  sensor->Init(trigPin, echoPin);
  // Initialize with a reading to warm up the sensor
  getValidDistance();
}

int SensorManager::getValidDistance() {
  int distances[MAX_READING_ATTEMPTS]; // Store all readings
  int validCount = 0;
  
  // Take multiple readings
  for (int i = 0; i < MAX_READING_ATTEMPTS; i++) {
    // Use a non-blocking approach for multiple readings
    int reading = static_cast<int>(sensor->Ranging());
    
    if (debugEnabled && btManagerInstance) {
      btManagerInstance->sendMessageF("Debug - Reading attempt %d: %dcm", i+1, reading);
    }
    
    // Check for valid readings
    if (reading >= MIN_VALID_DISTANCE && reading < MAX_VALID_DISTANCE) {
      distances[validCount++] = reading;
    }
    
    // Smaller delay between readings
    delay(10);
  }
  
  // If no valid readings, handle sensor issues
  if (validCount == 0) {
    consecutiveFailedReadings++;
    
    if (debugEnabled && btManagerInstance) {
      btManagerInstance->sendMessageF("Debug - No valid readings (%d consecutive failures). Check connections.", 
                                    consecutiveFailedReadings);
    }
    
    // If we have repeated failures, alert but use last known valid distance if available
    if (consecutiveFailedReadings >= 5) {
      if (btManagerInstance) {
        btManagerInstance->sendMessage("WARNING: Ultrasonic sensor may be disconnected or malfunctioning");
      }
      
      // If we have a previous valid reading, use it with an added safety margin
      // otherwise return a default safe value
      return (lastValidDistance > 0) ? lastValidDistance / 2 : FALLBACK_DISTANCE;
    }
    
    return FALLBACK_DISTANCE; // Return a large value to prevent false obstacle detection
  }
  
  // Reset consecutive failures on successful read
  consecutiveFailedReadings = 0;
  
  // With multiple readings, return the median (more robust against outliers)
  if (validCount > 1) {
    // Sort the array
    for (int i = 0; i < validCount - 1; i++) {
      for (int j = i + 1; j < validCount; j++) {
        if (distances[i] > distances[j]) {
          // Swap
          int temp = distances[i];
          distances[i] = distances[j];
          distances[j] = temp;
        }
      }
    }
    
    // Store and return the median value
    lastValidDistance = distances[validCount / 2];
    return lastValidDistance;
  }
  
  // Store and return the single valid reading
  lastValidDistance = distances[0];
  return lastValidDistance;
}

bool SensorManager::checkForObstacles(unsigned long currentTime) {
  static int lastDistance = FALLBACK_DISTANCE; // Start with a large value
  static unsigned long lastFullCheckTime = 0;
  
  if (!avoidanceEnabled) {
    return false;
  }
  
  // Quick check based on timing
  if (currentTime - lastObstacleCheck < OBSTACLE_CHECK_INTERVAL) {
    return false;
  }
  
  lastObstacleCheck = currentTime;
  
  // Adaptive timing: if clear path, check less frequently
  if (lastDistance > 100 && (currentTime - lastFullCheckTime < 1000)) {
    // Gradually increase the check interval based on distance
    unsigned long adjustedInterval = map(lastDistance, 100, 300, 1000, 2000);
    if (currentTime - lastFullCheckTime < adjustedInterval) {
      return false;
    }
  }
  
  // Get a valid distance reading
  lastDistance = getValidDistance();
  lastFullCheckTime = currentTime;
  
  if (debugEnabled && btManagerInstance) {
    btManagerInstance->sendMessageF("Debug - Current distance: %dcm", lastDistance);
  }
  
  // Return true if an obstacle is detected within range (defined in config.h)
  return (lastDistance > MIN_VALID_DISTANCE && lastDistance <= OBSTACLE_DETECTION_DISTANCE);
}

void SensorManager::setAvoidanceEnabled(bool enabled) {
  avoidanceEnabled = enabled;
  if (debugEnabled && btManagerInstance) {
    btManagerInstance->sendMessageF("Obstacle avoidance %s", enabled ? "enabled" : "disabled");
  }
}

void SensorManager::setDebugEnabled(bool enabled) {
  debugEnabled = enabled;
}

bool SensorManager::isAvoidanceEnabled() const {
  return avoidanceEnabled;
}

bool SensorManager::isDebugEnabled() const {
  return debugEnabled;
}