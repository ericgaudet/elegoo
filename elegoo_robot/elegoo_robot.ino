// WWFIRST 2021 Elegoo Robot Challenge
// Eric Gaudet
//
// Max robot size W x L x H (starting): 7.65" x 13.91" x 10" (19.43cm x 35.33cm x 25.4cm), current 18.1 x 33.5 x 22.6cm
// Max robot size W x L x H (game): 10" x 16" x unlimited (25.4cm x 40.64cm), current 18.1 x 36 x 36cm

#include <Servo.h>
#include "Drivetrain.h"
#include "DriverStation.h"
#include "UltrasonicSensor.h"


// Add-on hardware configuration
#define GRIPPER_SERVO_PIN   9
#define ELEVATOR_SERVO_PIN  10

// Controller Settings
#define JOYSTICK_DEADBAND   8

// Button IDs, from idx 0 (Logitech F310/F710)
//   A, B, X, Y, LB, RB, LT, RT, 
//   BACK, START, L3, R3, D-Up, D-Down, D-Left, D-Right
#define GRIPPER_CLOSE_BTN   1
#define GRIPPER_OPEN_BTN    2
#define CMD_GRAB_2ND_CUP    3


// Create hardware objects
Drivetrain drivetrain;  // DC motors, etc.
DriverStation ds;       // Joystick/controller and game flow
Servo gripperServo;
Servo elevatorServo;
UltrasonicSensor ultrasonic;

// Globals
bool firstTimeInAuto = true;
bool commandRunning = false;


void setup() {
  Serial.begin( 115200 );
  Serial.println( "Elegoo Robot v2.4" );
  gripperServo.attach( GRIPPER_SERVO_PIN );
  gripperServo.write(0);
  elevatorServo.attach( ELEVATOR_SERVO_PIN );
  elevatorServo.write(90);
}


// Autonomous mode
// Called 10 times per second
void autonomous() {
//  if(firstTimeInAuto) {
//    firstTimeInAuto = false;
//    Serial.println("Driving for 200mm");
//    drivetrain.autoDistance(200);
//  }
//
//  drivetrain.updateAuto();
}

// Teleop mode
// Called 10 times per second
void teleop() {
  if(!commandRunning) {
    // Drive according to joystick positions
    // LY is used for forward/backward drive speed
    // RX is used for turning speed
    int drivePower = ds.getLY();
    int rotatePower = ds.getRX();
  
    // Count small joystick values as 0
    if(drivePower > -JOYSTICK_DEADBAND && drivePower < JOYSTICK_DEADBAND) {
      drivePower = 0;
    }
    if(rotatePower > -JOYSTICK_DEADBAND && rotatePower < JOYSTICK_DEADBAND) {
      rotatePower = 0;
    }
    drivetrain.drive(drivePower, rotatePower);
  }

  // Check for buttons that trigger a group of commands
  if(!commandRunning) {
    if(ds.getButton(CMD_GRAB_2ND_CUP)) {
      // Setup the command
      // commandRunning = true;
    }
  }
  
  // Elevator
  int servoSpeed;
  int lt = ds.getLTrig();
  int rt = ds.getRTrig();
  // Map throttle values to servo speeds
  // - LT and RT run from 0..255
  // - Use LT to lower the elevator (LT 0..255 = servo to 90..180)
  // - Use RT to raise the elevator (RT 0..255 = servo to 90..0)
  if(lt > 0) {
    // Lower elevator
    servoSpeed = lt * 90;
    servoSpeed >>= 8;
    servoSpeed = 90 + servoSpeed;   
  }
  else if(rt > 0) {
    // Raise elevator
    servoSpeed = rt * 90;
    servoSpeed >>= 8;
    servoSpeed = 90 - servoSpeed;
  }
  else {
    servoSpeed = 90;  // Full-stop on continuous servo
  }
  elevatorServo.write(servoSpeed);

  // These buttons can interrupt commands
  // Gripper
  if(ds.getButton(GRIPPER_OPEN_BTN)) {
    gripperServo.write(0);
    // Interrupt command if running
  }
  else if(ds.getButton(GRIPPER_CLOSE_BTN)) {
    gripperServo.write(60);
    // Interrupt command if running
  }
}


void loop() {
  // Update the Driver Station state and check if new data has been received (10 times/second)
  if(ds.bUpdate()) {
    // Act based on game state
    switch(ds.getGameState()) {
      case ePreGame:
      case ePostGame:
        // During Pre and Post game, the Elegoo should not move!
        drivetrain.setPower(0, 0);
        // Reset for auto
        firstTimeInAuto = true;
        break;
      case eAutonomous:
        // Handle Autonomous mode
        autonomous();
        break;
      case eTeleop:
        // Handle telop mode
        teleop();
        break;
    }
  }
}


void commandHandler() {
  // Grab 2nd Cup
  // - Open gripper (first cup falls into second cup)
  // - Wait 0.5s
  // - Back-up 30mm (so elevator doesn't hit cups)
  // - Lower elevator to pick-up-height
  // - Drive forward 30mm
  // - Close gripper
  // - Wait 0.5s
  // - Raise elevator to max height
}
