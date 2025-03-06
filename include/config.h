#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// LED pin definitions
#define LEFT_LED 12  // Left LED for movement and hazard indication
#define RIGHT_LED 2  // Right LED for connection status

// Timeout and interval constants
#define BT_TIMEOUT 60000  // Bluetooth timeout (60 seconds)
#define CONNECTION_BLINK_INTERVAL 500  // Blink every 500ms when not connected
#define MOVEMENT_BLINK_INTERVAL 300    // Standard movement blink interval
#define FAST_BLINK_INTERVAL 150        // Fast blink for turning
#define OBSTACLE_BLINK_INTERVAL 100    // Very fast blink for obstacle detection
#define OBSTACLE_CHECK_INTERVAL 200    // Check for obstacles every 200ms
#define MIN_VALID_DISTANCE 2           // Ignore readings below this value (cm)
#define MAX_VALID_DISTANCE 400         // Maximum valid reading distance (cm)
#define MAX_READING_ATTEMPTS 3         // Number of attempts to get valid reading
#define FALLBACK_DISTANCE 1000         // Default distance when readings fail
#define OBSTACLE_DETECTION_DISTANCE 25 // Distance at which to detect obstacles (cm)

// Ultrasonic sensor pins
#define ULTRASONIC_TRIG_PIN 13
#define ULTRASONIC_ECHO_PIN 14

// Vehicle speeds
#define DEFAULT_SPEED 150
#define MIN_SPEED 50       // Minimum allowed speed
#define MAX_SPEED 255      // Maximum allowed speed (PWM max)
#define TURN_SPEED 180

// Movement types (from vehicle.h, included here for reference)
// enum Movement { Stop, Forward, Backward, Clockwise, Contrarotate };

// LED status codes
enum LedStatus {
  LED_IDLE,
  LED_FORWARD,
  LED_BACKWARD,
  LED_TURNING,
  LED_OBSTACLE,
  LED_ERROR
};

// Bluetooth device name
#define BT_DEVICE_NAME "test-bench"

#endif