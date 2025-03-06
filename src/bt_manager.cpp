#include "../include/bt_manager.h"
#include <stdarg.h>

BtManager::BtManager() {
  isConnected = false;
  lastActivityTime = 0;
}

void BtManager::init(const String& name) {
  deviceName = name;
  
  // Start the Bluetooth serial interface with the device name
  serialBT.begin(deviceName);
  
  // Delete all bonded devices for fresh pairing
  serialBT.deleteAllBondedDevices();
  
  Serial.printf("Bluetooth device \"%s\" started\n", deviceName.c_str());
}

void BtManager::processBtInput(unsigned long currentTime, String& inputBuffer) {
  // Check if there's any BT data available
  if (serialBT.available()) {
    // Activity detected - update connection status
    isConnected = true;
    lastActivityTime = currentTime;
    
    // Read all available data
    while (serialBT.available()) {
      char inChar = (char)serialBT.read();
      inputBuffer += inChar;
    }
  }
}

void BtManager::sendMessage(const String& message) {
  // Send to serial console with prefix
  Serial.println("[BT] " + message);
  
  // Send to Bluetooth if we think we're connected
  if (isConnected) {
    serialBT.println(message);
    lastActivityTime = millis();
  }
}

void BtManager::sendMessageF(const char* format, ...) {
  char buffer[128]; // May need adjustment
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  
  // Send to serial console with prefix
  Serial.print("[BT] ");
  Serial.println(buffer);
  
  // Send to Bluetooth if connected
  if (isConnected) {
    serialBT.println(buffer);
    lastActivityTime = millis();
  }
}

void BtManager::updateConnectionStatus(unsigned long currentTime) {
  // Check for timeout
  if (isConnected && (currentTime - lastActivityTime > BT_TIMEOUT)) {
    isConnected = false;
  }
}

bool BtManager::isDeviceConnected() const {
  return isConnected;
}

unsigned long BtManager::getLastActivityTime() const {
  return lastActivityTime;
}

void BtManager::sendPing() {
  if (isConnected) {
    serialBT.println("ping");
  }
}