#include "ultrasonic.h"
#include "Arduino.h"

void ultrasonic::Init(int trigPin, int echoPin)
{
    _trigPin = trigPin;
    _echoPin = echoPin;
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);
    
    // Ensure trigger is LOW at start
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);
    
    // Perform an initial reading to stabilize the sensor
    Ranging();
}

float ultrasonic::Ranging()
{
    // Make multiple attempts in case of error
    const int maxAttempts = 2;
    float distance = 0;
    
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        // Clear the trigger
        digitalWrite(_trigPin, LOW);
        delayMicroseconds(5);
        
        // Set trigger HIGH for 10 microseconds
        digitalWrite(_trigPin, HIGH);
        delayMicroseconds(10);
        digitalWrite(_trigPin, LOW);
        
        // Read the echo pulse with a timeout
        // 30ms timeout corresponds to ~5 meters which is a reasonable maximum
        unsigned long duration = pulseIn(_echoPin, HIGH, 30000);
        
        // Calculate distance if we received a valid pulse
        if (duration > 0) {
            // Convert to distance in cm (speed of sound = 343m/s)
            distance = duration * 0.0343 / 2;
            
            // If we got a reasonable reading, return it
            if (distance > 1 && distance < 400) {
                return distance;
            }
        }
        
        // Short delay before next attempt
        delay(5);
    }
    
    // Return the last calculated distance, even if invalid
    // This allows the calling function to decide how to handle errors
    return distance;
}