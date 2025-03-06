#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "config.h"
#include "../src/lib/ultrasonic/ultrasonic.h"

// Add these to config.h or define them here
#ifndef OBSTACLE_DETECTION_DISTANCE
#define OBSTACLE_DETECTION_DISTANCE 25  // cm
#endif

#ifndef MAX_VALID_DISTANCE
#define MAX_VALID_DISTANCE 400  // Maximum valid distance reading (cm)
#endif

#ifndef FALLBACK_DISTANCE
#define FALLBACK_DISTANCE 1000  // Distance to return when sensor fails (cm)
#endif

class SensorManager {
  private:
    ultrasonic* sensor;
    bool debugEnabled;
    unsigned long lastObstacleCheck;
    bool avoidanceEnabled;
    int consecutiveFailedReadings;
    int lastValidDistance;
    
  public:
    SensorManager();
    
    ~SensorManager();
    
    // Initialize the ultrasonic sensor
    void init(int trigPin, int echoPin);
    
    // Get a valid distance reading from the ultrasonic sensor
    int getValidDistance();
    
    // Check for obstacles and return true if one is detected
    bool checkForObstacles(unsigned long currentTime);
    
    // Enable/disable obstacle avoidance
    void setAvoidanceEnabled(bool enabled);
    
    // Enable/disable debug mode
    void setDebugEnabled(bool enabled);
    
    // Get obstacle avoidance state
    bool isAvoidanceEnabled() const;
    
    // Get debug mode state
    bool isDebugEnabled() const;
};

#endif