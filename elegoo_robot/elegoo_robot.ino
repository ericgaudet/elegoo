#include <Servo.h>
#include "elegoo-car.h"
#include "DriverStation.h"

// Simple Arduino Sketch that uses the Sample Driver Station.
// Load this sketch and drive the Elegoo around using an XBox Controller.

// Additional pinouts
#define GRIPPER_SERVO_PIN 9

// Create the ElegooCar object named myCar
ElegooCar myCar;
// Create the DriverStation object named ds
DriverStation ds;

#define AUTO_FWD_SPEED 156
#define FORWARD_SPEED 255
#define TURN_SPEED 64

#define DEADBAND 16 // Joystick values close to 0 that we'll ignore

// Create hardware objects
Servo gripperServo;
Servo elevatorServo;

void setup() {
  // put your setup code here, to run once:
  Serial.begin( 115200 );
  Serial.println( "Ready..." );
  gripperServo.attach( GRIPPER_SERVO_PIN );
  elevatorServo.attach( c_u8ServoPin );
}

// autonomous is called 10 times per second during Autonomous mode.
// this function must not block as new data is received every 100ms
//
// you could implement line following here...
void autonomous() {
  int curTime = ds.getStateTimer();

  // drive forward for 1000ms (1s)
  if( curTime < 1000 )
    myCar.setSpeed( FORWARD_SPEED, FORWARD_SPEED );

  // for next 500 ms turn
  else if( curTime < 1500 )
    myCar.setSpeed( -TURN_SPEED, TURN_SPEED );

  // drive straight again for 1000ms
  else if( curTime < 2500 )
    myCar.setSpeed( FORWARD_SPEED, FORWARD_SPEED );

  // stop after 2.5 seconds
  else
    myCar.setSpeed( 0, 0 );
}

// teleop function is called every time there is new data from the DriverStation
// this function must not block as new data is received every 100ms
void teleop() {
  // get the Right Y axis and use this as the "forward speed" for the robot
  int fwd = ds.getLY();

  // get the Right X axis and use it as the rate of turn
  int turn = ds.getRX();

  // Apply the turn adding the turn rate to the left wheels
  // and subtracting it from the right wheels
  int left = fwd + turn;
  int right = fwd - turn;

  // If the power to left or right wheel is too low, the motors can't
  // turn, so set them to 0 in the 'dead band'
  if( left > -DEADBAND && left < DEADBAND )
    left = 0;
  if( right > -DEADBAND && right < DEADBAND )
    right = 0;

  // Now tell the Elegoo how fast to turn the left and right wheels
  myCar.setSpeed( left, right );       

  // This is where you would likely add your code to control your attachment
  // As an example, we will set the Ultrasonic Range Finder direction based on
  // the Left stick X value

  // Elevator
#if 0 // Not done
  int servoSpeed = 90;  // Full-stop on continuous servo
  int lt = ds.getLTrig();
  int rt = ds.getRTrig();
  if( lt > 0 ) {
    // 0 = 90 deg, 255 = 0 deg, so 90/256 and inverted
    servoSpeed = lt * 90;
    servoSpeed >>= 8;
    servoSpeed = 90 - servoSpeed;   
  }
  else if( rt > 0 ) {
    servoSpeed = rt * 90;
    servoSpeed >>= 8;
    servoSpeed = 90 + servoSpeed;
  }
  else {
    servoSpeed = 0;
  }
  elevatorServo.write( servoSpeed );

  // Gripper
  if( ds.getButton(0) ) {
    gripperServo.write( 10 );
  }
  else if( ds.getButton(2) ) {
    gripperServo.write( 45 );
  }
#endif

//  Serial.print(ds.getLTrig());Serial.print(" ");
//  Serial.print(ds.getRTrig());Serial.print(" ");
//  Serial.print(ds.getButton(0));Serial.print(" ");
//  Serial.println(ds.getButton(2));
}

void loop() {
  // Update the Elegoo Car state
  int res = myCar.u16Update();

  // update the DriverStation class - this will check if there is new data from the
  // DriverStation application.
  // ds.bUpdate() returns true if new data has been received (10 times/second)
  if( ds.bUpdate() ) {
    // now, handle the driver station data depending on what game state we are in
    switch( ds.getGameState() ) {
      case ePreGame:
      case ePostGame:
        // During Pre and Post game, the Elegoo should not move!
        myCar.setSpeed( 0, 0 );
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

  // do other updates that need to happen more frequently than 10 times per second here...
  // e.g. checking limit switches...

  if( ds.getGameState() == eAutonomous ) {
    // Could do line-following here
  }
}
