#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "config.h"
#include "movement_controller.h"
#include "sensor_manager.h"
#include "bt_manager.h"

class CommandProcessor {
  private:
    MovementController* movementCtrl;
    SensorManager* sensorMgr;
    BtManager* btMgr;
    
    // Command type enumeration for more efficient parsing
    enum CommandType {
      CMD_UNKNOWN,
      CMD_FORWARD,
      CMD_BACKWARD,
      CMD_STOP,
      CMD_TURN,
      CMD_SPEED,
      CMD_DISTANCE,
      CMD_AVOID,
      CMD_DEBUG,
      CMD_HELP,
      CMD_PING,
      CMD_STATUS
    };
    
    // Structure to hold parsed command data
    struct ParsedCommand {
      CommandType type;
      int param1;
      int param2;
      bool flagValue;
    };
    
    // Parse a command string into a more usable structure
    ParsedCommand parseCommand(const String& cmd);
    
    // Helper method to send responses through the appropriate channel
    void sendResponse(const String& message);
    
  public:
    CommandProcessor(MovementController* moveCtrl, SensorManager* sensMgr, BtManager* bluetoothMgr);
    
    // Process a command string
    void processCommand(const String& command);
    
    // Print help information
    void printHelpInfo();
    
    // Process serial input
    void processSerialInput(String& inputBuffer);
};

#endif // COMMAND_PROCESSOR_H