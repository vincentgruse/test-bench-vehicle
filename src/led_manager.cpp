#include "../include/led_manager.h"

LedManager::LedManager() {
  rightLedState = false;
  leftLedState = false;
  currentLeftLedStatus = LED_IDLE;
  blinkPhase = 0;
  lastRightBlinkTime = 0;
  lastLeftBlinkTime = 0;
}

void LedManager::init() {
  pinMode(LEFT_LED, OUTPUT);
  pinMode(RIGHT_LED, OUTPUT);
  digitalWrite(LEFT_LED, LOW);   // Start with LEDs off
  digitalWrite(RIGHT_LED, LOW);
}

void LedManager::updateStatus(unsigned long currentTime, bool isConnected) {
  // Right LED - Connection Status
  if (isConnected) {
    // Connected - solid right LED
    digitalWrite(RIGHT_LED, HIGH);
    rightLedState = true;
  } else {
    // Not connected - blinking right LED
    if (currentTime - lastRightBlinkTime >= CONNECTION_BLINK_INTERVAL) {
      lastRightBlinkTime = currentTime;
      rightLedState = !rightLedState;
      digitalWrite(RIGHT_LED, rightLedState);
    }
  }
  
  // Left LED - Movement and Hazard Status
  switch (currentLeftLedStatus) {
    case LED_IDLE:
      // Idle - LED off
      digitalWrite(LEFT_LED, LOW);
      leftLedState = false;
      break;
      
    case LED_FORWARD:
      // Forward - solid LED
      digitalWrite(LEFT_LED, HIGH);
      leftLedState = true;
      break;
      
    case LED_BACKWARD:
      // Backward - slow blinking
      if (currentTime - lastLeftBlinkTime >= MOVEMENT_BLINK_INTERVAL) {
        lastLeftBlinkTime = currentTime;
        leftLedState = !leftLedState;
        digitalWrite(LEFT_LED, leftLedState);
      }
      break;
      
    case LED_TURNING:
      // Turning - fast blinking
      if (currentTime - lastLeftBlinkTime >= FAST_BLINK_INTERVAL) {
        lastLeftBlinkTime = currentTime;
        leftLedState = !leftLedState;
        digitalWrite(LEFT_LED, leftLedState);
      }
      break;
      
    case LED_OBSTACLE:
      // Obstacle detected - double flash pattern
      if (currentTime - lastLeftBlinkTime >= OBSTACLE_BLINK_INTERVAL) {
        lastLeftBlinkTime = currentTime;
        
        // Double flash pattern sequencing
        blinkPhase = (blinkPhase + 1) % 6;
        
        // ON-OFF-ON-OFF-OFF-OFF pattern (double flash)
        if (blinkPhase == 0 || blinkPhase == 2) {
          digitalWrite(LEFT_LED, HIGH);
          leftLedState = true;
        } else {
          digitalWrite(LEFT_LED, LOW);
          leftLedState = false;
        }
      }
      break;
      
    case LED_ERROR:
      // Error - alternating LEDs
      if (currentTime - lastLeftBlinkTime >= FAST_BLINK_INTERVAL) {
        lastLeftBlinkTime = currentTime;
        leftLedState = !leftLedState;
        digitalWrite(LEFT_LED, leftLedState);
        digitalWrite(RIGHT_LED, !leftLedState);
      }
      break;
  }
}

void LedManager::setLeftLedStatus(LedStatus status) {
  currentLeftLedStatus = status;
  blinkPhase = 0; // Reset blink phase for patterns
  
  // Immediate update for some states
  if (status == LED_IDLE) {
    digitalWrite(LEFT_LED, LOW);
    leftLedState = false;
  } else if (status == LED_FORWARD) {
    digitalWrite(LEFT_LED, HIGH);
    leftLedState = true;
  }
}

LedStatus LedManager::getCurrentLeftLedStatus() const {
  return currentLeftLedStatus;
}