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
#if 0
  #define TRACE(x)  Serial.println(x)
#else
  #define TRACE(x)
#endif


// Controller Settings
#define JOYSTICK_DEADBAND   8

// Button IDs, from idx 0 (Logitech F310/F710)
#define DRP_AND_2ND_CUP_BTN 0   // A (Green, bottom)
#define GRIPPER_CLOSE_BTN   1   // B (Red, right)
#define GRIPPER_OPEN_BTN    2   // X (Blue, left)
#define GRAB_2ND_CUP_BTN    3   // Y (Yellow, top)
#define ELEVATOR_TO_BOT_BTN 4   // LB
#define ELEVATOR_TO_TOP_BTN 5   // RB
//#define 6   // LT - used as throttle
//#define 7   // RT - used as throttle
#define ROTATE_TEST_BTN     8   // BACK
#define DRIVE_TEST_BTN      9   // START
#define CANCEL1_BTN         10  // L3
#define CANCEL2_BTN         11  // R3
#define GRAB_1ST_CUP_BTN    12  // D-Up
//#define 13  // D-Down
#define ALIGN_TO_CUP_L_BTN  14  // D-Left
#define ALIGN_TO_CUP_R_BTN  15  // D-Right

// Command settings
#define MAX_SEARCH_ROTATE_DEG   135
#define MAX_CUP_DISTANCE_MM     300
#define CUP_PICKUP_DISTANCE_MM  50
#define CUP_BACKOFF_DISTANCE_MM 30


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
  bool isRunning;             // Whether or not a command is running
  void (*handleCmdSeq)(void); // Function pointer to the command handler
  int param;                  // Parameter for function (use varies by handler)
  int curStep;                // Current step in the command sequence
  int lastAlignDistance;      // Holds the distance from the last align command
} g_cmdSeqCtrl;


////////////////////////////////////////////////////////////////////
// Arduino setup function.  Called on power-up.
void setup() {
  g_cmdSeqCtrl.isRunning = false;
  drivetrain.init();
  elevator.init();
  gripper.init();
  
  Serial.begin( 115200 );
  Serial.println( "Elegoo Robot v4.0" );
}


////////////////////////////////////////////////////////////////////
// Main Arduino loop function.  Called continuously
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


////////////////////////////////////////////////////////////////////
// Autonomous mode
// Called 10 times per second
void autonomous() {

}


////////////////////////////////////////////////////////////////////
// Teleop mode
// Called 10 times per second
// In this function, look for changes in the controls and trigger the appropriate reaction.
// Do the control stuff in the main loop as it runs much more frequently (quicker reaction time).
void teleop() {
#ifdef DRIVE_ONLY
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
#else
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
    else if(ds.getButton(ALIGN_TO_CUP_L_BTN)) {
      g_cmdSeqCtrl.handleCmdSeq = &handleAlignToCup; //handleScanAndAlignToCup
      g_cmdSeqCtrl.param = 0; // 0 = left-hand turn
      g_cmdSeqCtrl.isRunning = true;
    }
    else if(ds.getButton(ALIGN_TO_CUP_R_BTN)) {
      g_cmdSeqCtrl.handleCmdSeq = &handleAlignToCup; //handleScanAndAlignToCup
      g_cmdSeqCtrl.param = 1; // 1 = right-hand turn
      g_cmdSeqCtrl.isRunning = true;
    }
    else if(ds.getButton(GRAB_1ST_CUP_BTN)) {
      g_cmdSeqCtrl.handleCmdSeq = &handle1stCupPickup;
      g_cmdSeqCtrl.isRunning = true;
    }
    else if(ds.getButton(DRP_AND_2ND_CUP_BTN)) {
      g_cmdSeqCtrl.handleCmdSeq = &handleDropAnd2ndCupPickup;
      g_cmdSeqCtrl.isRunning = true;
    }
    else if(ds.getButton(GRAB_2ND_CUP_BTN)) {
      g_cmdSeqCtrl.handleCmdSeq = &handle2ndCupPickup;
      g_cmdSeqCtrl.param = 0; // 0 = left-hand turn
      g_cmdSeqCtrl.isRunning = true;
    }
    else if(ds.getButton(DRIVE_TEST_BTN)) {
      g_cmdSeqCtrl.handleCmdSeq = &handleDriveTest;
      g_cmdSeqCtrl.isRunning = true;
    }
    else if(ds.getButton(ROTATE_TEST_BTN)) {
      g_cmdSeqCtrl.handleCmdSeq = &handleRotateTest;
      g_cmdSeqCtrl.isRunning = true;
    }
    
    // If a new command has started, start running it now
    if(g_cmdSeqCtrl.isRunning) {
      g_cmdSeqCtrl.curStep = 0;
      g_cmdSeqCtrl.handleCmdSeq();
    }
      
  }
#endif
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
// Align the robot to a cup within range.  Can rotate left of right.
void handleAlignToCup() {
  static bool foundPossibleCup = false;
  
  if(g_cmdSeqCtrl.isRunning) {
    switch(g_cmdSeqCtrl.curStep) {
    case 0:
      g_cmdSeqCtrl.lastAlignDistance = 0;
      
      // Raise elevator
      elevator.setPower(256);
      g_cmdSeqCtrl.curStep++;
      break;

    case 1:
      // Check if elevator is raised
      if(elevator.isAtUpperLimit()) {
        elevator.setPower(0);

        // Start turning
        // param = 0 means left turn, 1 means right turn
        drivetrain.autoRotate((g_cmdSeqCtrl.param == 0) ? -MAX_SEARCH_ROTATE_DEG : MAX_SEARCH_ROTATE_DEG);
        g_cmdSeqCtrl.curStep++;
      }
      break;
    
    case 2:
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        // Quit if we've turned too far and haven't found a cup
        g_cmdSeqCtrl.isRunning = false;
      }
      else {
        // Check if we've found a cup close by
        int distance = ultrasonic.getDistanceMm();
        logDistance(distance);
        if((distance > 0) && (distance < MAX_CUP_DISTANCE_MM)) {
          // Don't jump at the first cup with think we see
          if(foundPossibleCup) {
            // This is the second time we've seen an object so assume it's not a glitch
            // Stop turning and calculate how far we are from the cup
            drivetrain.abortAuto();
            TRACE(distance);
            g_cmdSeqCtrl.lastAlignDistance = distance - CUP_PICKUP_DISTANCE_MM;
            foundPossibleCup = false;
            // Done
            g_cmdSeqCtrl.isRunning = false;
          }
          else {
            foundPossibleCup = true;
            TRACE(distance);
          }
        }
        else {
          foundPossibleCup = false;
        }
      }
      break;
    }
  }
  
  // If command finished or was stopped, clean up
  if(!g_cmdSeqCtrl.isRunning) {
    drivetrain.abortAuto();
    drivetrain.drive(0, 0);
    elevator.setPower(0);
    printDistanceLog();
    TRACE("CMD DONE: Align");
  }
}


////////////////////////////////////////////////////////////////////
// Scan an arc for a cup and align to the middle of the detection
// range.  Can rotate left of right.
void handleScanAndAlignToCup() {
  static bool foundPossibleCup = false;
  
  if(g_cmdSeqCtrl.isRunning) {
    switch(g_cmdSeqCtrl.curStep) {
    case 0:
      g_cmdSeqCtrl.lastAlignDistance = 0;
      resetDistanceLog();
      
      // Raise elevator
      elevator.setPower(256);
      g_cmdSeqCtrl.curStep++;
      break;

    case 1:
      // Check if elevator is raised
      if(elevator.isAtUpperLimit()) {
        elevator.setPower(0);

        // Scan a full arc
        // param = 0 means left turn, 1 means right turn
        drivetrain.autoRotate((g_cmdSeqCtrl.param == 0) ? -MAX_SEARCH_ROTATE_DEG : MAX_SEARCH_ROTATE_DEG);
        g_cmdSeqCtrl.curStep++;
      }
      break;
    
    case 2:
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        // Get the angle to the centre of the cup and get the distance the cup is at
        int cupAngle = calcCupAngle(&g_cmdSeqCtrl.lastAlignDistance);
        // Turn to that angle
        drivetrain.autoRotate((g_cmdSeqCtrl.param == 0) ? cupAngle : -cupAngle);
        g_cmdSeqCtrl.curStep++;
      }
      else {
        // Measure the distance and record it
        int distance = ultrasonic.getDistanceMm();
        logDistance(distance);
      }
      break;

    case 3:
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
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
    printDistanceLog();
    TRACE("CMD DONE: Scan and Align");
  }
}


////////////////////////////////////////////////////////////////////
// Drive up to the first cup and grip it.  Distance is based on the
// align command's measured distance.
void handle1stCupPickup() {
  static int distanceToDrive = 0;
  static bool foundPossibleCup = false;
  
  if(g_cmdSeqCtrl.isRunning) {
    switch(g_cmdSeqCtrl.curStep) {
    case 0:
      // Open the gripper
      gripper.open();
      g_cmdSeqCtrl.curStep++;
      // Wait until gripper opens
      timer.set(500);
      break;
      
    case 1:
      // Check if we've waited long enough
      if(timer.isExpired()) {
        // Lower elevator
        elevator.setPower(-256);
        g_cmdSeqCtrl.curStep++;
      }
      break;
    
    case 2:
      // Check if elevator is lowered
      if(elevator.isAtLowerLimit()) {
        elevator.setPower(0);
        // Drive to the cup using pre-calculated distance
        drivetrain.autoDistance(g_cmdSeqCtrl.lastAlignDistance);
        g_cmdSeqCtrl.curStep++;
      }
      break;

    case 3:
      // Update the drivetrain state machine and check if the move is done
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        // Close the gripper to grab the cup and then wait for things to stabilize
        gripper.close();
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
    printDistanceLog();
    TRACE("CMD DONE: 1st Cup");
  }
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
void handleDropAnd2ndCupPickup() {
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


////////////////////////////////////////////////////////////////////
// Same as the previous 2nd-cup grab function but advances based on
// the align command's distance measurement
void handle2ndCupPickup() {  
  static bool foundPossibleCup = false;
  
  // Make sure the sequence hasn't been cancelled
  if(g_cmdSeqCtrl.isRunning) {
    switch(g_cmdSeqCtrl.curStep) {
   case 0:
      // Raise elevator
      elevator.setPower(256);
      g_cmdSeqCtrl.curStep++;
      break;

    case 1:
      // Check if elevator is raised
      if(elevator.isAtUpperLimit()) {
        elevator.setPower(0);
 
        // Drive to the cup
        drivetrain.autoDistance(g_cmdSeqCtrl.lastAlignDistance);
        g_cmdSeqCtrl.curStep++;
      }
      break;

    case 2:
      // Update the drivetrain state machine and check if the move is done
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        // Open gripper to let 1st cup drop into 2nd one
        gripper.open();
        // Wait until gripper opens and cup falls
        timer.set(500);
        g_cmdSeqCtrl.curStep++;
      }
      break;
      
    case 3:
      // Check if we've waited long enough
      if(timer.isExpired()) {
        // Back up a bit (so elevator doesn't hit cups on way down)
        drivetrain.autoDistance(-CUP_BACKOFF_DISTANCE_MM);
        g_cmdSeqCtrl.curStep++;
      }
      break;
      
    case 4:
      // Update the drivetrain state machine and check if the move is done
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        // Lower elevator to pick-up-height
        elevator.setPower(-256);
        g_cmdSeqCtrl.curStep++;
      }
      break;
      
    case 5:
      // Check if elevator is lowered
      if(elevator.isAtLowerLimit()) {
        // Stop the elevator and drive forward 30mm
        elevator.setPower(0);
        drivetrain.autoDistance(CUP_BACKOFF_DISTANCE_MM);
        g_cmdSeqCtrl.curStep++;
      }
      break;
      
    case 6:
      // Update the drivetrain state machine and check if the move is done
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        // Close the gripper to grab the cups and done
        gripper.close();
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
    printDistanceLog();
    TRACE("CMD DONE: 2nd cup rot");
  }
}


////////////////////////////////////////////////////////////////////
// Drive half the field to check that the encoders are correct
#define HALF_FIELD_DISTANCE_MM  905
void handleDriveTest() {
   // Make sure the sequence hasn't been cancelled
  if(g_cmdSeqCtrl.isRunning) {
    switch(g_cmdSeqCtrl.curStep) {
    case 0:
        // Drive the set distance
        drivetrain.autoDistance(HALF_FIELD_DISTANCE_MM);
        g_cmdSeqCtrl.curStep++;
      break;

    case 1:
      // Update the drivetrain state machine and check if the move is done
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
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
    TRACE("CMD DONE: Drive Test");
  }
}


////////////////////////////////////////////////////////////////////
// Rotate a set amount to test the encoders
#define ROTATE_TEST_DEG 90
void handleRotateTest() {
  if(g_cmdSeqCtrl.isRunning) {
    switch(g_cmdSeqCtrl.curStep) {
    case 0:
      // Start turning
      // param = 0 means left turn, 1 means right turn
      drivetrain.autoRotate(ROTATE_TEST_DEG);
      g_cmdSeqCtrl.curStep++;
      break;
    
    case 1:
      drivetrain.updateAuto();
      if(drivetrain.isAutoIdle()) {
        // Done turning
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
    printDistanceLog();
    TRACE("CMD DONE: Rotate Test");
  }
}


////////////////////////////////////////////////////////////////////
// Save a set of distances for debugging purposes
#define DISTANCE_LOG_LENGTH 200
int distanceLog[DISTANCE_LOG_LENGTH];
int distanceLogIdx = 0;
void logDistance(int distance) {
  distanceLog[distanceLogIdx] = distance;
  if(++distanceLogIdx >= DISTANCE_LOG_LENGTH) {
    distanceLogIdx = 0;
    TRACE("Distance Log Full");
  }
}


////////////////////////////////////////////////////////////////////
// Print out the distance log
void printDistanceLog() {
  for(int idx; idx < distanceLogIdx; idx++) {
    Serial.print(distanceLog[idx]);
    Serial.print(",");
  }
  Serial.println("");
  resetDistanceLog();
}


////////////////////////////////////////////////////////////////////
// Reset the log index
void resetDistanceLog() {
  distanceLogIdx = 0;
}


////////////////////////////////////////////////////////////////////
// Find the centre of the set of closet measurements.  Return the 
// angle to counter rotate to get there and the distance to that cup
#define NEW_CUP_DIST_MM 50
int calcCupAngle(int *pDistance) {
  // Assume we haven't found anything close enough
  int angle = 0;
  if(pDistance) *pDistance = 0;

  int scanningState = 0;
  int cupStartIdx = -1;
  int cupEndIdx = -1;
  for(int idx = 0; idx <= distanceLogIdx; idx++) {
    if(distanceLog[idx] == -1) {
      // Could be too close or too far.  Don't know so ignore
      continue;
    }
    switch(scanningState) {
    case 0:
      // Start by trying to find something that's below threshold
      if(distanceLog[idx] < MAX_CUP_DISTANCE_MM) {
        cupStartIdx = idx;
        scanningState++;
      }
      break;
      
    case 1:
      // We have found the start of a cup.  Now check for two things:
      // - A closer cup (140 vs 150)
      // - The end of that cup (200 vs 150)
      if((distanceLog[cupStartIdx] - distanceLog[idx]) > NEW_CUP_DIST_MM) {
        // Found a closer cup, reset the start index
        cupStartIdx = idx;
      }
      else if((distanceLog[idx] - distanceLog[cupStartIdx]) > NEW_CUP_DIST_MM) {
        // Found the end of the cup
        cupEndIdx = idx - 1;
        scanningState++;
      }

    case 2:
      // TODO:  Break out of the for loop
      break;
    }
  }

  // Act on what we found
  if(cupStartIdx == -1) {
    // Didn't find a cup
    return 0;
  }
  
  if(cupEndIdx == -1) {
    // Didn't find the end of the cup so assume the last entry is the end
    cupEndIdx = distanceLogIdx;
    Serial.println("No cup end found");
  }

  // Calculate the angle to the centre of the cup
  // angle-to-cup = ((last-idx - cup-centre-idx) / last-idx) * overall-scan-angle
  int centreIdx = (cupEndIdx - cupStartIdx + 1) / 2 + cupStartIdx;
  angle = ((distanceLogIdx - centreIdx) / (float)distanceLogIdx) * MAX_SEARCH_ROTATE_DEG;
  if(pDistance) {
    *pDistance = distanceLog[centreIdx];
  }

  Serial.print("Cup S");
  Serial.print(cupStartIdx);
  Serial.print(" E");
  Serial.print(cupEndIdx);
  Serial.print(" C");
  Serial.print(centreIdx);
  Serial.print(" L");
  Serial.print(distanceLogIdx);
  Serial.print(" A");
  Serial.print(angle);
  Serial.print(" D");
  Serial.println(*pDistance);

  return angle;
}
