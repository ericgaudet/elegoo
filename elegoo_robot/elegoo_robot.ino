// WWFIRST 2021 Elegoo Robot Challenge
// Eric Gaudet
//
// Max robot size W x L x H (starting): 7.65" x 13.91" x 10" (19.43cm x 35.33cm x 25.4cm), current 18.1 x 33.5 x 22.6cm
// Max robot size W x L x H (game): 10" x 16" x unlimited (25.4cm x 40.64cm), current 18.1 x 36 x 36cm

#include "Drivetrain.h"
#include "DriverStation.h"
#include "Gripper.h"
#include "Elevator.h"
#include "Timer.h"
#include "UltrasonicSensor.h"


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
Elevator elevator;
Gripper gripper;
Timer timer;
UltrasonicSensor ultrasonic;


// Globals
bool firstTimeInAuto = true;
bool commandRunning = false;


void setup() {
  Serial.begin( 115200 );
  Serial.println( "Elegoo Robot v3.0" );
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
  int servoPower;
  int lt = ds.getLTrig();
  int rt = ds.getRTrig();
  // Map throttle values to servo speeds
  // - LT and RT run from 0..255
  // - Use LT to lower the elevator
  // - Use RT to raise the elevator
  if(lt > 0) {
    // Lower elevator
    servoPower = -lt;  
  }
  else if(rt > 0) {
    // Raise elevator
    servoPower = rt;
  }
  else {
    servoPower = 0;
  }
  elevator.setPower(servoPower);

  // These buttons can interrupt commands
  // Gripper
  if(ds.getButton(GRIPPER_OPEN_BTN)) {
    gripper.open();
    // Interrupt command if running
  }
  else if(ds.getButton(GRIPPER_CLOSE_BTN)) {
    gripper.close();
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


#define COMMAND_2ND_CUP 1
void commandHandler() {
  static int commandId = COMMAND_2ND_CUP;  // TODO: move these
  static int commandStage = 0;

  // Determine which command we're running
  switch(commandId) {
    
  case COMMAND_2ND_CUP:
    // Walk through the grab-2nd-cup command's stages
    // PRECONDITIONS:
    // - First cup in gripper
    // - Elevator up
    // - First cup positioned over second cup
    
    // TODO:  Break out into separate handle2ndCupCommand()?
    switch(commandStage) {
    case 0:
      // Open gripper (first cup falls into second cup)
      gripper.open();
      commandStage++;
      // Wait until gripper opens and cup falls
      timer.set(500);
      break;
    case 1:
      if(timer.isExpired()) {
        commandStage++;
        // Back up 30mm (so elevator doesn't hit cups on way down)
        drivetrain.autoDistance(-30);
      }
      break;
    case 2:
      // Update the drivetrain state machine and check if the move is done
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        commandStage++;
        // Lower elevator to pick-up-height
        elevator.setPower(-256);
      }
      break;
    case 3:
      // Check if elevator is lowered
      if(elevator.isAtLowerLimit()) {
        elevator.setPower(0);
        commandStage++;
        // Drive forward 30mm
        drivetrain.autoDistance(30);
      }
      break;
    case 4:
      // Update the drivetrain state machine and check if the move is done
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        commandStage++;
        // Close the gripper to grab the cups and then wait for things to stabilize
        gripper.close();
        timer.set(500);
      }
      break;
    case 5:
      if(timer.isExpired()) {
        commandStage++;
        // Raise elevator to platform-drop-off-height
        elevator.setPower(256);
      }
      break;
    case 6:
      // Check if elevator is raised
      if(elevator.isAtUpperLimit()) {
        elevator.setPower(0);
        // Command done
        commandRunning = 0;
        commandId = 0;
        commandStage = 0;
      }
      break;
    }
    break;    
  }
}
