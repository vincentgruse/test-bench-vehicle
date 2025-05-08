#ifndef MOVEMENT_CONTROLLER_H
#define MOVEMENT_CONTROLLER_H

#include "config.h"
#include "../src/lib/vehicle/vehicle.h"
#include "led_manager.h"
#include "bt_manager.h"
#include "serial_manager.h"

// State machine for non-blocking avoidance maneuver
enum AvoidanceState { 
  AVOID_IDLE, 
  AVOID_BACKING, 
  AVOID_TURNING, 
  AVOID_COMPLETE 
};

class MovementController {
  private:
    vehicle* car;
    LedManager* ledManager;
    unsigned long timedMoveEnd;
    int currentSpeed;
    
    // For non-blocking avoidance maneuver
    AvoidanceState avoidanceState;
    unsigned long stateChangeTime;
    
  public:
    MovementController(LedManager* ledMgr);
    
    // Initialize the car
    void init();
    
    // Move forward (optionally for specified duration)
    void moveForward(int durationSeconds = 0);
    
    // Move backward (optionally for specified duration)
    void moveBackward(int durationSeconds = 0);
    
    // Move forward at specified speed (optionally for specified duration)
    void moveForwardWithSpeed(int speed, int durationSeconds = 0);
    
    // Move backward at specified speed (optionally for specified duration)
    void moveBackwardWithSpeed(int speed, int durationSeconds = 0);
    
    // Set the global speed
    void setSpeed(int speed);
    
    // Get the current speed
    int getSpeed() const;
    
    // Stop movement
    void stop();
    
    // Turn by specified degrees (positive for right, negative for left)
    void turnByDegrees(int degrees);
    
    // Perform obstacle avoidance maneuver
    void performAvoidanceManeuver();
    
    // Update the avoidance maneuver state machine
    void updateAvoidanceManeuver(unsigned long currentTime);
    
    // Check and handle timed movements
    void checkTimedMovements(unsigned long currentTime);
    
    // Get the current timed move end time
    unsigned long getTimedMoveEnd() const;
    
    // Cancel any timed movement
    void cancelTimedMovement();
};

#endif