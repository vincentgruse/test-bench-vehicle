#ifndef MESSAGE_MANAGER_H
#define MESSAGE_MANAGER_H

#include <Arduino.h>

class MessageManager {
  public:
    // Send a simple message - always uses Serial
    static void send(const String& message) {
      Serial.println(message);
    }
    
    // Send a formatted message - always uses Serial
    static void sendF(const char* format, ...) {
      char buffer[128]; // Buffer for formatted string
      va_list args;
      va_start(args, format);
      vsnprintf(buffer, sizeof(buffer), format, args);
      va_end(args);
      
      Serial.println(buffer);
    }
    
    // For compatibility - always returns true for "connected"
    static bool isConnected() {
      return true;
    }
};

#endif