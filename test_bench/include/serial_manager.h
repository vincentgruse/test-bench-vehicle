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
      isConnected = checkConnection();  // Initialize with actual status
    }
    
    // Check if USB Serial is actually connected
    bool checkConnection() {
      return Serial;  // On ESP32, Serial evaluates to true if connected to a host
    }
    
    // Update connection status
    void updateConnectionStatus() {
      isConnected = checkConnection();
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
    
    // Return actual USB connection status
    bool isDeviceConnected() const {
      return isConnected;
    }
};

#endif