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


// For debugging purposes
#if 1
  #define TRACE(x)  Serial.println(x)
#else
  #define TRACE(x)
#endif


// Controller Settings
#define JOYSTICK_DEADBAND   8


// Button IDs, from idx 0 (Logitech F310/F710)
#define PICKUP_CUP_BTN      0   // A (Green, bottom)
#define GRIPPER_CLOSE_BTN   1   // B (Red, right)
#define GRIPPER_OPEN_BTN    2   // X (Blue, left)
#define GRAB_2ND_CUP_BTN    3   // Y (Yellow, top)
#define ELEVATOR_TO_BOT_BTN 4   // LB
#define ELEVATOR_TO_TOP_BTN 5   // RB
//#define 6   // LT - used as throttle
//#define 7   // RT - used as throttle
//#define 8   // BACK
//#define 9   // START
#define CANCEL1_BTN         10  // L3
#define CANCEL2_BTN         11  // R3
//#define 12  // D-Up
//#define 13  // D-Down
//#define 14  // D-Left
//#define 15  // D-Right


// Create hardware objects
Drivetrain drivetrain;  // DC motors, etc.
DriverStation ds;       // Joystick/controller and game flow
Elevator elevator;
Gripper gripper;
Timer timer;
UltrasonicSensor ultrasonic;


// Globals
bool g_firstTimeInAuto = true;
struct CommandSequenceController {
  bool isRunning;
  void (*handleCmdSeq)(void);
  int curStep;  
} g_cmdSeqCtrl;


void setup() {
  g_cmdSeqCtrl.isRunning = false;
  drivetrain.drive(0, 0);
  elevator.init();
  gripper.init();
  
  Serial.begin( 115200 );
  Serial.println( "Elegoo Robot v3.1" );
}


void loop() {
  // Update the Driver Station state and check if new data has been received (10 times/second)
  if(ds.bUpdate()) {
    // Act based on game state
    switch(ds.getGameState()) {
    case ePreGame:
    case ePostGame:
      // Reset for auto
      g_firstTimeInAuto = true;
      
      // Stop any running commands
      if(g_cmdSeqCtrl.isRunning) {
        g_cmdSeqCtrl.isRunning = false;
        g_cmdSeqCtrl.handleCmdSeq();
      }
      
      // During Pre and Post game, the Elegoo should not move!
      drivetrain.setPower(0, 0);
      elevator.setPower(0);
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

  // If a command sequence is running, service it now
  if(g_cmdSeqCtrl.isRunning) {
    g_cmdSeqCtrl.handleCmdSeq();
  }
}


// Autonomous mode
// Called 10 times per second
void autonomous() {
//  if(g_firstTimeInAuto) {
//    g_firstTimeInAuto = false;
//    Serial.println("Driving for 200mm");
//    drivetrain.autoDistance(200);
//  }
//
//  drivetrain.updateAuto();
}


// Teleop mode
// Called 10 times per second
// In this function, look for changes in the controls and trigger the appropriate reaction.
// Do the control stuff in the main loop as it runs much more frequently (quicker reaction time).
void teleop() {
  // Check for the command-cancel buttons
  if(ds.getButton(CANCEL1_BTN) || ds.getButton(CANCEL2_BTN)) {
    // Make sure there's something to cancel
    if(g_cmdSeqCtrl.isRunning) {
      // Setting 'running' to false and calling the handler will cause it to stop gracefully
      g_cmdSeqCtrl.isRunning = false;
      g_cmdSeqCtrl.handleCmdSeq();
    } 
  }
  
  // Only respond to these inputs if no command is running
  if(!g_cmdSeqCtrl.isRunning) {
    // /Let the joysticks control the robot
    
    // Drive
    // - LY is used for forward/backward drive speed
    // - RX is used for turning speed
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

    // Elevator
    // - LT is used to lower
    // - RT is used to raise
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

    // Now react to button presses
    // Single-action buttons
    if(ds.getButton(GRIPPER_OPEN_BTN)) {
      gripper.open();
    }
    else if(ds.getButton(GRIPPER_CLOSE_BTN)) {
      gripper.close();
    }

    // Command-sequence buttons
    // For now, don't let anything else happen while a command sequence is running.
    // In the future, could improve so you could, for example, keep driving while
    // the elevator is moving to the bottom.
    if(ds.getButton(ELEVATOR_TO_BOT_BTN)) {
      g_cmdSeqCtrl.handleCmdSeq = &handleElevatorToBottom;
      g_cmdSeqCtrl.isRunning = true;
    }
    else if(ds.getButton(ELEVATOR_TO_TOP_BTN)) {
      g_cmdSeqCtrl.handleCmdSeq = &handleElevatorToTop;
      g_cmdSeqCtrl.isRunning = true;
    }
    else if(ds.getButton(PICKUP_CUP_BTN)) {
      
    }
    else if(ds.getButton(GRAB_2ND_CUP_BTN)) {
      g_cmdSeqCtrl.handleCmdSeq = &handle2ndCupPickup;
      g_cmdSeqCtrl.isRunning = true;
    }
    
    // If a new command has started, start running it now
    if(g_cmdSeqCtrl.isRunning) {
      g_cmdSeqCtrl.curStep = 0;
      g_cmdSeqCtrl.handleCmdSeq();
    }
      
  }
}


////////////////////////////////////////////////////////////////////
// Moves the elevator down until it hits the lower limit switch
void handleElevatorToBottom() {
  // Make sure the sequence hasn't been cancelled
  if(g_cmdSeqCtrl.isRunning) {
    switch(g_cmdSeqCtrl.curStep) {
    case 0:
      // Start lowering the elevator
      TRACE("CMD: E to bot");
      drivetrain.drive(0, 0);
      elevator.setPower(-256);
      g_cmdSeqCtrl.curStep++;
      // no break;
    case 1:
      // Check if the lower limit switch has been triggered
      if(elevator.isAtLowerLimit()) {
        g_cmdSeqCtrl.isRunning = false;
      }
      break;
    }
  }

  // If command finished or was stopped, clean up
  if(!g_cmdSeqCtrl.isRunning) {
    elevator.setPower(0);
    TRACE("CMD DONE: E to bot");
  }        
}


////////////////////////////////////////////////////////////////////
// Moves the elevator up until it hits the upper limit switch
void handleElevatorToTop() {
  // Make sure the sequence hasn't been cancelled
  if(g_cmdSeqCtrl.isRunning) {
    switch(g_cmdSeqCtrl.curStep) {
    case 0:
      // Start raising the elevator
      TRACE("CMD: E to top");
      drivetrain.drive(0, 0);
      elevator.setPower(256);
      g_cmdSeqCtrl.curStep++;
      // no break;
    case 1:
      // Check if the lower limit switch has been triggered
      if(elevator.isAtUpperLimit()) {
        g_cmdSeqCtrl.isRunning = false;
      }
      break;
    }
  }

  // If command finished or was stopped, clean up
  if(!g_cmdSeqCtrl.isRunning) {
    elevator.setPower(0);
    TRACE("CMD DONE: E to top");
  }        
}


////////////////////////////////////////////////////////////////////
// Rotate the robot left and right using the ultrasonic sensor to 
// detect the cup position.  Once it's in front of the robot, drive
// up to it and grip it.
void handlePickupCup() {
  // TODO: Implement
}


////////////////////////////////////////////////////////////////////
// Once the first cup is in the gripper, the elevator is all the way
// up and the first cup is positioned over the second cup:
// - Open the gripper
// - Wait a bit
// - Backup a bit
// - Lower the elevator
// - Drive up to the stack of cups
// - Close the gripper
// - Wait a bit
// - Raise the elevator
void handle2ndCupPickup() {
  // Make sure the sequence hasn't been cancelled
  if(g_cmdSeqCtrl.isRunning) {
    switch(g_cmdSeqCtrl.curStep) {
    case 0:
      // Open gripper (first cup falls into second cup)
      TRACE("CMD: 2nd cup");
      drivetrain.drive(0, 0);
      elevator.setPower(0);
      gripper.open();
      g_cmdSeqCtrl.curStep++;
      
      // Wait until gripper opens and cup falls
      timer.set(500);
      break;
      
    case 1:
      // Check if we've waited long enough
      if(timer.isExpired()) {
        // Back up a bit (so elevator doesn't hit cups on way down)
        drivetrain.autoDistance(-30);
        g_cmdSeqCtrl.curStep++;
      }
      break;
      
    case 2:
      // Update the drivetrain state machine and check if the move is done
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        // Lower elevator to pick-up-height
        elevator.setPower(-256);
        g_cmdSeqCtrl.curStep++;
      }
      break;
      
    case 3:
      // Check if elevator is lowered
      if(elevator.isAtLowerLimit()) {
        // Stop the elevator and drive forward 30mm
        elevator.setPower(0);
        drivetrain.autoDistance(30);
        g_cmdSeqCtrl.curStep++;
      }
      break;
      
    case 4:
      // Update the drivetrain state machine and check if the move is done
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        // Close the gripper to grab the cups and then wait for things to stabilize
        gripper.close();
        timer.set(500);
        g_cmdSeqCtrl.curStep++;
      }
      break;
      
    case 5:
      // Check if we've waited long enough
      if(timer.isExpired()) {
        // Raise elevator to platform-drop-off-height
        elevator.setPower(256);
        g_cmdSeqCtrl.curStep++;
      }
      break;
      
    case 6:
      // Check if elevator is raised
      if(elevator.isAtUpperLimit()) {
        g_cmdSeqCtrl.isRunning = false;
      }
      break;
    }
  }

  // If command finished or was stopped, clean up
  if(!g_cmdSeqCtrl.isRunning) {
    drivetrain.abortAuto();
    drivetrain.drive(0, 0);
    elevator.setPower(0);
    TRACE("CMD DONE: 2nd cup");
  }
}
