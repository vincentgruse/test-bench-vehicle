#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include "config.h"

class LedManager {
  private:
    bool rightLedState;
    bool leftLedState;
    LedStatus currentLeftLedStatus;
    int blinkPhase;  // For complex patterns
    unsigned long lastRightBlinkTime;
    unsigned long lastLeftBlinkTime;
    
  public:
    LedManager();
    
    // Initialize the LED pins
    void init();
    
    // Update LED status based on current time and connection status
    void updateStatus(unsigned long currentTime, bool isConnected);
    
    // Set the left LED status based on vehicle state
    void setLeftLedStatus(LedStatus status);
    
    // Get the current left LED status
    LedStatus getCurrentLeftLedStatus() const;
};

#endif