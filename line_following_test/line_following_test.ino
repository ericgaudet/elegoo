// Using the light sensors to follow a black line on a white background
//
// How it works:
// - When the middle sensor finds the black line (sensor reports 0) drive forward.
// - If the middle sensor isn't on the black line, check the side sensors to see
//   which direction we need to turn to get back on track.
// - Don't forget to adjust the sensor sensativity (white and blue potentiometer)
//   to work with your lighting consditions.
// - This is a simplistic example.  You could improve on the logic to get better performance.

// Simple drivetrain implementation for the Elegoo car
#include "drive.h"

#define LIGHT_SENSOR_LEFT_PIN   A0  // Usually 2
#define LIGHT_SENSOR_MIDDLE_PIN A1  // Usually 4
#define LIGHT_SENSOR_RIGHT_PIN  A2  // Usually 10


void setup() {
  Serial.begin(115200);
  Serial.println("Line Follower (line sensors) Test v1");

  pinMode(LIGHT_SENSOR_LEFT_PIN, INPUT);
  pinMode(LIGHT_SENSOR_MIDDLE_PIN, INPUT);
  pinMode(LIGHT_SENSOR_RIGHT_PIN, INPUT);

  driveSetup();
}

void loop() {
  if(digitalRead(LIGHT_SENSOR_MIDDLE_PIN) == 0) {
    // Black line is in the middle, keep going
    driveForward();
  }
  else if(digitalRead(LIGHT_SENSOR_LEFT_PIN) == 0) {
    // Black line is under the left sensor to go left to bring it to the middle
    driveLeft();
  }
  else if(digitalRead(LIGHT_SENSOR_RIGHT_PIN) == 0) {
    // Black line is under the right sensor to go right to bring it to the middle
    driveRight();
  }
  else {
    // We've lost the line so give up
    // Could try backing up instead
    driveStop();
  }


}
