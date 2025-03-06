#include "../include/command_processor.h"

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
  
  btMgr->sendMessage("Command received: " + cmd);
  
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
        btMgr->sendMessage("Invalid speed value. Please specify a positive number.");
      }
      break;
      
    case CMD_DISTANCE:
      {
        int validDistance = sensorMgr->getValidDistance();
        btMgr->sendMessage("Current distance: " + String(validDistance) + " cm");
      }
      break;
      
    case CMD_AVOID:
      sensorMgr->setAvoidanceEnabled(parsed.flagValue);
      btMgr->sendMessage("Obstacle avoidance " + String(parsed.flagValue ? "enabled" : "disabled"));
      break;
      
    case CMD_DEBUG:
      sensorMgr->setDebugEnabled(parsed.flagValue);
      btMgr->sendMessage("Debug mode " + String(parsed.flagValue ? "enabled" : "disabled"));
      break;
      
    case CMD_PING:
      btMgr->sendMessage("pong");
      break;
      
    case CMD_STATUS:
      if (btMgr->isDeviceConnected()) {
        btMgr->sendMessage("Bluetooth: Connected");
      } else {
        btMgr->sendMessage("Bluetooth: Disconnected");
      }
      btMgr->sendMessage("Current speed: " + String(movementCtrl->getSpeed()));
      btMgr->sendMessage("Obstacle avoidance: " + String(sensorMgr->isAvoidanceEnabled() ? "Enabled" : "Disabled"));
      btMgr->sendMessage("Debug mode: " + String(sensorMgr->isDebugEnabled() ? "Enabled" : "Disabled"));
      break;
      
    default:
      btMgr->sendMessage("Unknown command. Type 'help' for available commands.");
      break;
  }
}

void CommandProcessor::printHelpInfo() {
  btMgr->sendMessage("Test-bench Car Control Commands:");
  btMgr->sendMessage("---------------------------");
  btMgr->sendMessage("Speed Control:");
  btMgr->sendMessage("  speed [value]: Set global speed (50-255)");
  btMgr->sendMessage("");
  btMgr->sendMessage("Movement Commands:");
  btMgr->sendMessage("  forward/f: Move forward at current speed");
  btMgr->sendMessage("  forward/f [seconds]: Move forward for specified seconds at current speed");
  btMgr->sendMessage("  forward/f [speed] [seconds]: Move forward at specific speed for specified seconds");
  btMgr->sendMessage("  backward/b: Move backward at current speed");
  btMgr->sendMessage("  backward/b [seconds]: Move backward for specified seconds at current speed");
  btMgr->sendMessage("  backward/b [speed] [seconds]: Move backward at specific speed for specified seconds");
  btMgr->sendMessage("  stop/s: Stop movement");
  btMgr->sendMessage("  turn X: Turn by X degrees (positive for right, negative for left)");
  btMgr->sendMessage("");
  btMgr->sendMessage("Sensor Commands:");
  btMgr->sendMessage("  distance: Report current distance from ultrasonic sensor");
  btMgr->sendMessage("  avoid on/off: Enable/disable obstacle avoidance");
  btMgr->sendMessage("  debug on/off: Enable/disable sensor debugging information");
  btMgr->sendMessage("");
  btMgr->sendMessage("Other Commands:");
  btMgr->sendMessage("  help: Show this help information");
  btMgr->sendMessage("  ping: Simple connectivity test");
  btMgr->sendMessage("  status: Show current system status (includes speed)");
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