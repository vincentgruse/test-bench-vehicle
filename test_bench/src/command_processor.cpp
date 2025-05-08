#include "../include/command_processor.h"
#include "../include/message_manager.h"

CommandProcessor::CommandProcessor(MovementController* moveCtrl, SensorManager* sensMgr, BtManager* bluetoothMgr) {
  movementCtrl = moveCtrl;
  sensorMgr = sensMgr;
  btMgr = bluetoothMgr;
}

CommandProcessor::ParsedCommand CommandProcessor::parseCommand(const String& cmd) {
  ParsedCommand result = {CMD_UNKNOWN, 0, 0, false};
  
  if (cmd.length() == 0) {
    return result;
  }
  
  // Create lowercase version of command for case-insensitive matching
  String lowerCmd = cmd;
  lowerCmd.toLowerCase();
  
  // Check for simple commands first
  if (lowerCmd.equals("help")) {
    result.type = CMD_HELP;
    return result;
  }
  else if (lowerCmd.equals("ping")) {
    result.type = CMD_PING;
    return result;
  }
  else if (lowerCmd.equals("status")) {
    result.type = CMD_STATUS;
    return result;
  }
  else if (lowerCmd.equals("distance")) {
    result.type = CMD_DISTANCE;
    return result;
  }
  else if (lowerCmd.equals("stop") || lowerCmd.equals("s")) {
    result.type = CMD_STOP;
    return result;
  }
  
  // Check for commands with no parameters
  if (lowerCmd.equals("forward") || lowerCmd.equals("f")) {
    result.type = CMD_FORWARD;
    return result;
  }
  
  if (lowerCmd.equals("backward") || lowerCmd.equals("b")) {
    result.type = CMD_BACKWARD;
    return result;
  }
  
  // For commands with parameters
  int spaceIndex = lowerCmd.indexOf(' ');
  if (spaceIndex <= 0) {
    return result; // No space found or space at beginning
  }
  
  String cmdName = lowerCmd.substring(0, spaceIndex);
  String params = lowerCmd.substring(spaceIndex + 1);
  
  if (cmdName.equals("forward") || cmdName.equals("f")) {
    result.type = CMD_FORWARD;
    
    // Check for second parameter
    int secondSpaceIndex = params.indexOf(' ');
    if (secondSpaceIndex > 0) {
      // Two parameters: speed and duration
      result.param1 = params.substring(0, secondSpaceIndex).toInt();
      result.param2 = params.substring(secondSpaceIndex + 1).toInt();
    } else {
      // One parameter: duration (using global speed)
      result.param1 = params.toInt();
    }
  }
  else if (cmdName.equals("backward") || cmdName.equals("b")) {
    result.type = CMD_BACKWARD;
    
    // Check for second parameter
    int secondSpaceIndex = params.indexOf(' ');
    if (secondSpaceIndex > 0) {
      // Two parameters: speed and duration
      result.param1 = params.substring(0, secondSpaceIndex).toInt();
      result.param2 = params.substring(secondSpaceIndex + 1).toInt();
    } else {
      // One parameter: duration (using global speed)
      result.param1 = params.toInt();
    }
  }
  else if (cmdName.equals("turn")) {
    result.type = CMD_TURN;
    result.param1 = params.toInt();
  }
  else if (cmdName.equals("speed")) {
    result.type = CMD_SPEED;
    result.param1 = params.toInt();
  }
  else if (cmdName.equals("avoid")) {
    result.type = CMD_AVOID;
    result.flagValue = params.equals("on");
  }
  else if (cmdName.equals("debug")) {
    result.type = CMD_DEBUG;
    result.flagValue = params.equals("on");
  }
  
  return result;
}

void CommandProcessor::processCommand(const String& command) {
  // Ignore empty commands
  if (command.length() == 0) {
    return;
  }
  
  String cmd = command;
  cmd.trim();
  
  // Use the appropriate messaging method based on where the command came from
  if (btMgr != nullptr && btMgr->isDeviceConnected()) {
    btMgr->sendMessage("Command received: " + cmd);
  } else {
    MessageManager::send("Command received: " + cmd);
  }
  
  ParsedCommand parsed = parseCommand(cmd);
  
  switch (parsed.type) {
    case CMD_HELP:
      printHelpInfo();
      break;
      
    case CMD_FORWARD:
      if (parsed.param2 > 0) {
        // Two parameters provided: specific speed and duration
        movementCtrl->moveForwardWithSpeed(parsed.param1, parsed.param2);
      } else if (parsed.param1 > 0) {
        // One parameter: use global speed with this duration
        movementCtrl->moveForward(parsed.param1);
      } else {
        // No parameters: move forward at global speed
        movementCtrl->moveForward();
      }
      break;
      
    case CMD_BACKWARD:
      if (parsed.param2 > 0) {
        // Two parameters provided: specific speed and duration
        movementCtrl->moveBackwardWithSpeed(parsed.param1, parsed.param2);
      } else if (parsed.param1 > 0) {
        // One parameter: use global speed with this duration
        movementCtrl->moveBackward(parsed.param1);
      } else {
        // No parameters: move backward at global speed
        movementCtrl->moveBackward();
      }
      break;
      
    case CMD_STOP:
      movementCtrl->stop();
      break;
      
    case CMD_TURN:
      movementCtrl->turnByDegrees(parsed.param1);
      break;
      
    case CMD_SPEED:
      if (parsed.param1 > 0) {
        movementCtrl->setSpeed(parsed.param1);
      } else {
        sendResponse("Invalid speed value. Please specify a positive number.");
      }
      break;
      
    case CMD_DISTANCE:
      {
        int validDistance = sensorMgr->getValidDistance();
        sendResponse("Current distance: " + String(validDistance) + " cm");
      }
      break;
      
    case CMD_AVOID:
      sensorMgr->setAvoidanceEnabled(parsed.flagValue);
      sendResponse("Obstacle avoidance " + String(parsed.flagValue ? "enabled" : "disabled"));
      break;
      
    case CMD_DEBUG:
      sensorMgr->setDebugEnabled(parsed.flagValue);
      sendResponse("Debug mode " + String(parsed.flagValue ? "enabled" : "disabled"));
      break;
      
    case CMD_PING:
      sendResponse("pong");
      break;
      
    case CMD_STATUS:
      // Send different status messages depending on connection type
      if (btMgr != nullptr && btMgr->isDeviceConnected()) {
        btMgr->sendMessage("Connection: Bluetooth");
      } else {
        MessageManager::send("Connection: USB Serial");
      }
      
      sendResponse("Current speed: " + String(movementCtrl->getSpeed()));
      sendResponse("Obstacle avoidance: " + String(sensorMgr->isAvoidanceEnabled() ? "Enabled" : "Disabled"));
      sendResponse("Debug mode: " + String(sensorMgr->isDebugEnabled() ? "Enabled" : "Disabled"));
      break;
      
    default:
      sendResponse("Unknown command. Type 'help' for available commands.");
      break;
  }
}

// Helper method to send responses through the appropriate channel
void CommandProcessor::sendResponse(const String& message) {
  if (btMgr != nullptr && btMgr->isDeviceConnected()) {
    btMgr->sendMessage(message);
  } else {
    MessageManager::send(message);
  }
}

void CommandProcessor::printHelpInfo() {
  sendResponse("Test-bench Car Control Commands:");
  sendResponse("---------------------------");
  sendResponse("Speed Control:");
  sendResponse("  speed [value]: Set global speed (50-255)");
  sendResponse("");
  sendResponse("Movement Commands:");
  sendResponse("  forward/f: Move forward at current speed");
  sendResponse("  forward/f [seconds]: Move forward for specified seconds at current speed");
  sendResponse("  forward/f [speed] [seconds]: Move forward at specific speed for specified seconds");
  sendResponse("  backward/b: Move backward at current speed");
  sendResponse("  backward/b [seconds]: Move backward for specified seconds at current speed");
  sendResponse("  backward/b [speed] [seconds]: Move backward at specific speed for specified seconds");
  sendResponse("  stop/s: Stop movement");
  sendResponse("  turn X: Turn by X degrees (positive for right, negative for left)");
  sendResponse("");
  sendResponse("Sensor Commands:");
  sendResponse("  distance: Report current distance from ultrasonic sensor");
  sendResponse("  avoid on/off: Enable/disable obstacle avoidance");
  sendResponse("  debug on/off: Enable/disable sensor debugging information");
  sendResponse("");
  sendResponse("Other Commands:");
  sendResponse("  help: Show this help information");
  sendResponse("  ping: Simple connectivity test");
  sendResponse("  status: Show current system status (includes speed)");
}

void CommandProcessor::processSerialInput(String& inputBuffer) {
  while (Serial.available() > 0) {
    char inChar = (char)Serial.read();
    if (inChar == '\n' || inChar == '\r') {
      if (inputBuffer.length() > 0) {
        processCommand(inputBuffer);
        inputBuffer = "";
      }
    } else {
      inputBuffer += inChar;
    }
  }
}