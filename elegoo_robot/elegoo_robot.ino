// WWFIRST 2021 Elegoo Robot Challenge
// Eric Gaudet
//
// Max robot size W x L x H (starting): 7.65" x 13.91" x 10" (19.43cm x 35.33cm x 25.4cm), current 18.1 x 33.5 x 22.6cm
// Max robot size W x L x H (game): 10" x 16" x unlimited (25.4cm x 40.64cm), current 18.1 x 36 x 36cm

#include <Servo.h>
#include "elegoo-car.h"
#include "DriverStation.h"


// Add-on hardware configuration
#define GRIPPER_SERVO_PIN   3
#define ELEVATOR_SERVO_PIN  10

// Other settings
#define AUTO_FWD_SPEED      156
#define FORWARD_SPEED       255
#define TURN_SPEED          64

#define JOYSTICK_DEADBAND   64


// Create hardware objects
ElegooCar myCar;      // DC motors, etc.
DriverStation ds;     // Joystick/controller and game flow
Servo gripperServo;
Servo elevatorServo;


void setup() {
  Serial.begin( 115200 );
  Serial.println( "Elegoo Robot v0.1" );
  gripperServo.attach( GRIPPER_SERVO_PIN );
  gripperServo.write(0);
  elevatorServo.attach( ELEVATOR_SERVO_PIN );
  elevatorServo.write(90);
}


// Autonomous mode
// Called 10 times per second
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

// Teleop mode
// Called 10 times per second
void teleop() {
  // Drive according to joystick positions
  // LY is used for forward/backward drive speed
  // RX is used for turning speed
  int drivePower = ds.getLY();
  int turnPower = ds.getRX();
  int leftPower = drivePower + turnPower;
  int rightPower = drivePower - turnPower;

  if(leftPower > -JOYSTICK_DEADBAND && leftPower < JOYSTICK_DEADBAND) {
    leftPower = 0;
  }
  if(rightPower > -JOYSTICK_DEADBAND && rightPower < JOYSTICK_DEADBAND) {
    rightPower = 0;
  }
  myCar.setSpeed( leftPower, rightPower );       

  // Elevator
  int servoSpeed;
  int lt = ds.getLTrig();
  int rt = ds.getRTrig();
  // Map throttle values to servo speeds
  // - LT and RT run from 0..255
  // - Use LT to lower the elevator (LT 0..255 = servo to 90..180)
  // - Use RT to raise the elevator (RT 0..255 = servo to 90..0)
  if( lt > 0 ) {
    // Lower elevator
    servoSpeed = lt * 90;
    servoSpeed >>= 8;
    servoSpeed = 90 + servoSpeed;   
  }
  else if( rt > 0 ) {
    // Raise elevator
    servoSpeed = rt * 90;
    servoSpeed >>= 8;
    servoSpeed = 90 - servoSpeed;
  }
  else {
    servoSpeed = 90;  // Full-stop on continuous servo
  }
  elevatorServo.write( servoSpeed );

  // Gripper
  if( ds.getButton(12) ) {
    gripperServo.write( 0 );
  }
  else if( ds.getButton(13) ) {
    gripperServo.write( 60 );
  }
}


void loop() {
  // Update the Elegoo Car state
  myCar.u16Update();

  // Update the Driver Station state and check if new data has been received (10 times/second)
  if( ds.bUpdate() ) {
    // Act based on game state
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
}
