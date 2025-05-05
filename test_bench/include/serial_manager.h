#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include "config.h"
#include <Arduino.h>

class SerialManager {
  private:
    bool isConnected;
    
  public:
    // Constructor
    SerialManager() {
      isConnected = true;  // USB Serial is always "connected" when powered
    }
    
    // Send message via Serial
    void sendMessage(const String& message) {
      Serial.println(message);
    }
    
    // Send formatted message via Serial
    void sendMessageF(const char* format, ...) {
      char buffer[128]; // Buffer for formatted string
      va_list args;
      va_start(args, format);
      vsnprintf(buffer, sizeof(buffer), format, args);
      va_end(args);
      
      Serial.println(buffer);
    }
    
    // Always return true for isDeviceConnected since it's a USB connection
    bool isDeviceConnected() const {
      return isConnected;
    }
};

#endif