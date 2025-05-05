#ifndef BT_MANAGER_H
#define BT_MANAGER_H

#include "config.h"
#include "BluetoothSerial.h"

class BtManager {
  private:
    BluetoothSerial serialBT;
    String deviceName;
    bool isConnected;
    unsigned long lastActivityTime;
    
  public:
    // Constructor
    BtManager();
    
    // Initialize Bluetooth with device name
    void init(const String& name);
    
    // Process incoming BT data
    void processBtInput(unsigned long currentTime, String& inputBuffer);
    
    // Send message via Bluetooth and Serial
    void sendMessage(const String& message);
    
    // Send formatted message via Bluetooth and Serial
    void sendMessageF(const char* format, ...);
    
    // Check connection status based on activity
    void updateConnectionStatus(unsigned long currentTime);
    
    // Get connection status
    bool isDeviceConnected() const;
    
    // Get time of last activity
    unsigned long getLastActivityTime() const;
    
    // Send a ping to check connection
    void sendPing();
};

#endif