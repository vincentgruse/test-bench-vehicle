#include "../include/movement_controller.h"
#include "../include/bt_manager.h"

// External reference to the BtManager for sending messages
extern BtManager* btManagerInstance;

MovementController::MovementController(LedManager* ledMgr) {
  car = new vehicle();
  ledManager = ledMgr;
  timedMoveEnd = 0;
  currentSpeed = DEFAULT_SPEED;  // Initialize with default speed
  avoidanceState = AVOID_IDLE;
  stateChangeTime = 0;
}

void MovementController::init() {
  car->Init();
}

// New methods for handling speed
void MovementController::setSpeed(int speed) {
  // Constrain speed within valid range
  if (speed < MIN_SPEED) {
    speed = MIN_SPEED;
  } else if (speed > MAX_SPEED) {
    speed = MAX_SPEED;
  }
  
  currentSpeed = speed;
  
  if (btManagerInstance) {
    btManagerInstance->sendMessage("Speed set to " + String(currentSpeed));
  }
}

int MovementController::getSpeed() const {
  return currentSpeed;
}

// Methods using the global speed setting
void MovementController::moveForward(int durationSeconds) {
  moveForwardWithSpeed(currentSpeed, durationSeconds);
}

void MovementController::moveBackward(int durationSeconds) {
  moveBackwardWithSpeed(currentSpeed, durationSeconds);
}

// Methods with explicit speed
void MovementController::moveForwardWithSpeed(int speed, int durationSeconds) {
  car->Move(Forward, speed);
  ledManager->setLeftLedStatus(LED_FORWARD);
  
  if (durationSeconds > 0) {
    timedMoveEnd = millis() + (durationSeconds * 1000);
    if (btManagerInstance) {
      btManagerInstance->sendMessage("Moving forward at speed " + String(speed) + " for " + String(durationSeconds) + " seconds");
    }
  } else {
    if (btManagerInstance) {
      btManagerInstance->sendMessage("Moving forward at speed " + String(speed));
    }
  }
}

void MovementController::moveBackwardWithSpeed(int speed, int durationSeconds) {
  car->Move(Backward, speed);
  ledManager->setLeftLedStatus(LED_BACKWARD);
  
  if (durationSeconds > 0) {
    timedMoveEnd = millis() + (durationSeconds * 1000);
    if (btManagerInstance) {
      btManagerInstance->sendMessage("Moving backward at speed " + String(speed) + " for " + String(durationSeconds) + " seconds");
    }
  } else {
    if (btManagerInstance) {
      btManagerInstance->sendMessage("Moving backward at speed " + String(speed));
    }
  }
}

void MovementController::stop() {
  car->Move(Stop, 0);
  ledManager->setLeftLedStatus(LED_IDLE);
  timedMoveEnd = 0;
  
  if (btManagerInstance) {
    btManagerInstance->sendMessage("Stopping");
  }
}

void MovementController::turnByDegrees(int degrees) {
  // Positive degrees for right turn, negative for left
  if (btManagerInstance) {
    btManagerInstance->sendMessage("Turning " + String(abs(degrees)) + " degrees " + (degrees > 0 ? "right" : "left"));
  }
  
  ledManager->setLeftLedStatus(LED_TURNING);
  
  // Stop any existing movement first
  car->Move(Stop, 0);
  delay(50);
  
  if (degrees > 0) {
    car->Move(Clockwise, TURN_SPEED);
  } else if (degrees < 0) {
    car->Move(Contrarotate, TURN_SPEED);
  } else {
    if (btManagerInstance) {
      btManagerInstance->sendMessage("No turn needed (0 degrees)");
    }
    return;
  }
  
  // IMPORTANT: Using blocking delay for precise turning
  // Calculate delay time based on degrees
  // Using 1500ms for 90 degrees
  int delayTime = abs(degrees) * 1500 / 90;
  
  // Debug message to verify calculation
  if (btManagerInstance) {
    btManagerInstance->sendMessageF("Turn time: %d ms for %d degrees", delayTime, abs(degrees));
  }
  
  // Use blocking delay for precise timing
  delay(delayTime);
  
  // Stop immediately after delay
  car->Move(Stop, 0);
  ledManager->setLeftLedStatus(LED_IDLE);
  
  if (btManagerInstance) {
    btManagerInstance->sendMessage("Turn complete");
  }
}

void MovementController::performAvoidanceManeuver() {
  // Set obstacle detection LED status
  ledManager->setLeftLedStatus(LED_OBSTACLE);
  
  // Start the avoidance maneuver state machine
  avoidanceState = AVOID_BACKING;
  car->Move(Backward, 150);
  stateChangeTime = millis() + 500; // Back up for 500ms
  
  if (btManagerInstance) {
    btManagerInstance->sendMessage("Starting avoidance maneuver");
  }
}

void MovementController::updateAvoidanceManeuver(unsigned long currentTime) {
  if (avoidanceState == AVOID_IDLE) {
    return;
  }
  
  if (currentTime >= stateChangeTime) {
    switch (avoidanceState) {
      case AVOID_BACKING:
        // Switch to turning state
        car->Move(Contrarotate, 180);
        avoidanceState = AVOID_TURNING;
        stateChangeTime = currentTime + 1000; // Turn for 1000ms
        break;
        
      case AVOID_TURNING:
        // Complete the maneuver
        car->Move(Stop, 0);
        avoidanceState = AVOID_IDLE;
        ledManager->setLeftLedStatus(LED_IDLE);
        
        if (btManagerInstance) {
          btManagerInstance->sendMessage("Avoidance maneuver complete");
        }
        break;
        
      default:
        avoidanceState = AVOID_IDLE;
        break;
    }
  }
}

void MovementController::checkTimedMovements(unsigned long currentTime) {
  if (timedMoveEnd > 0 && currentTime >= timedMoveEnd) {
    car->Move(Stop, 0);
    if (btManagerInstance) {
      btManagerInstance->sendMessage("Timed movement complete");
    }
    timedMoveEnd = 0;
    ledManager->setLeftLedStatus(LED_IDLE);
  }
}

unsigned long MovementController::getTimedMoveEnd() const {
  return timedMoveEnd;
}

void MovementController::cancelTimedMovement() {
  timedMoveEnd = 0;
}