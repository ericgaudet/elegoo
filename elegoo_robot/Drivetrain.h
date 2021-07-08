// Drivetrain subsystem
// Includes wheel motor control (2 left, 2 right), wheel encorder management and line following sensors.
#ifndef DRIVETRAIN_H
#define DRIVETRAIN_H

#include "RobotMap.h"
#include "TankDriveSide.h"
#include "WheelEncoder.h"

// Constants
#define AUTO_STRAIGHT_POWER         144
#define AUTO_TURN_POWER             224
#define LINE_FOLLOW_STRAIGHT_POWER  160
#define LINE_FOLLOW_TURN_POWER      160
#define TICKS_TO_MM_FACTOR          (178/905.0) //(109/280.0)
#define WHEEL_BASE_MM               145.0

enum States {
  idle = 0,
  straight,
  rotate,
  driveToLine,
  lineFollower
};

class Drivetrain {
private:
  class TankDriveSide m_leftSide;
  class TankDriveSide m_rightSide;
  class WheelEncoder m_leftEncoder;
  //class WheelEncoder m_rightEncoder;
  int m_leftEncoderCount;
  //int m_rightEncoderCount;
  int m_leftTargetTicks;
  //int m_rightTargetTicks;
  enum States m_state;
  
public:
  // Constructor
  Drivetrain(): 
    m_leftSide(),
    m_rightSide(),
    m_leftEncoder()
    /*m_rightEncoder()*/ {}


  ////////////////////////////////////////////////////////////////////
  // Initializer (constructor wasn't a good place to do this)
  void init() {
    m_leftSide.init(L298_ENA_PIN, L298_IN1_PIN, L298_IN2_PIN);
    m_rightSide.init(L298_ENB_PIN, L298_IN4_PIN, L298_IN3_PIN);
    m_leftEncoder.init(LEFT_WHEEL_ENCODER_PIN, true);
//    m_rightEncoder.init(RIGHT_WHEEL_ENCODER_PIN, false);

    pinMode(LINE_LEFT_PIN, INPUT);
    pinMode(LINE_MIDDLE_PIN, INPUT);
    pinMode(LINE_RIGHT_PIN, INPUT);
    
    m_leftEncoder.setTicksToDistanceFactor(TICKS_TO_MM_FACTOR);
//    m_rightEncoder.setTicksToDistanceFactor(TICKS_TO_MM_FACTOR);
    m_leftTargetTicks = 0;
//    m_rightTargetTicks = 0;
    m_state = idle;
    setPower(0, 0);
  }

  ////////////////////////////////////////////////////////////////////
  // Set left and right side power (0..255)
  void setPower(int left, int right) {
    m_leftSide.setPower(left);
    m_rightSide.setPower(right);
    m_leftEncoder.setDirectionForward(left > 0 ? true : false);
//    m_rightEncoder.setDirectionForward(right > 0 ? true : false);
  }

  ////////////////////////////////////////////////////////////////////
  // Tank drive wrapper for setPower (powers: -254..254)
  void drive(int drivePower, int rotatePower) {
    bool drivePowerNegative = false;
    long leftPower;
    long rightPower;
  
    // Do the math on the magnitude of the drive power
    if(drivePower < 0) {
      drivePowerNegative = true;
      drivePower = -drivePower;
    }
    
    // If turning, reduce that side's power, eventually going in reverse
    // Assume power ranges of -256 to 256 to simplify calculations
    if(rotatePower < 0) {
      leftPower = (((254 + (long)rotatePower) * drivePower) / 127) - drivePower; // divide by 128
      rightPower = drivePower;
    }
    else if(rotatePower > 0) {
      leftPower = drivePower;
      rightPower = (((254 - (long)rotatePower) * drivePower) / 127) - drivePower;
    }
    else {
      leftPower = drivePower;
      rightPower = drivePower;
    }

    // All the math above is done for positive drivePower.  Reverse power if drive was negative.
    if(drivePowerNegative) {
      leftPower = -leftPower;
      rightPower = -rightPower;
    }
    
    setPower(leftPower, rightPower);
  }

  ////////////////////////////////////////////////////////////////////
  // Update the auto-drive state machine (for motion when not using joysticks)
  void updateAuto() {
    int ticks = 0;
    
    switch(m_state) {
    case idle:
      break;
      
    case straight:
      // Check if we've gotten to the target distance (take into account the direction)
      ticks = m_leftEncoder.getDistanceInTicks();
      if((m_leftTargetTicks >= 0 && (ticks >= m_leftTargetTicks)) ||
         (m_leftTargetTicks < 0 && (ticks <= m_leftTargetTicks))) {
        setPower(0, 0);
        m_state = idle;
        Serial.print("Ending straight drive (");
        Serial.print(ticks);
        Serial.print(" of ");
        Serial.print(m_leftTargetTicks);
        Serial.print(" P:");
        Serial.print(getPollCount());
        Serial.println(")");
      }
      break;
      
    case rotate:
      // Check if we've gotten to the target distance (take into account the direction)
      ticks = m_leftEncoder.getDistanceInTicks();
      if((m_leftTargetTicks >= 0 && (ticks >= m_leftTargetTicks)) ||
         (m_leftTargetTicks < 0 && (ticks <= m_leftTargetTicks))) {
        setPower(0, 0);
        m_state = idle;
        Serial.print("Ending rotate (");
        Serial.print(ticks);
        Serial.print(" of ");
        Serial.print(m_leftTargetTicks);
        Serial.print(" P:");
        Serial.print(getPollCount());
        Serial.println(")");
      }
      break;

    case driveToLine:
      if(digitalRead(LINE_MIDDLE_PIN) == 0) {
        setPower(0, 0);
        m_state = idle;
      }
      break;

    case lineFollower:
      autoLineFollow();
      break;

    }
  }

  ////////////////////////////////////////////////////////////////////
  // Returns true of no automation is running
  bool isAutoIdle() {
    return (m_state == idle) ? true : false;
  }

  ////////////////////////////////////////////////////////////////////
  // Aborts an auto maneuver
  void abortAuto() {
    m_state = idle;
    setPower(0, 0);
  }

  ////////////////////////////////////////////////////////////////////
  // Auto Distance (in mm)
  void autoDistance(int distance) {
    // Immediate stop condition
    if(distance == 0) {
      m_state = idle;
      setPower(0, 0);
      return;
    }
    
    // Set target distance in encoder ticks
    m_leftTargetTicks = m_leftEncoder.getNumTicksInDistance(distance);
//    m_rightTargetTicks = m_rightEncoder.getNumTicksInDistance(distance);

    // Reset the encoders and set the direction of motion
    m_leftEncoder.reset();
//    m_rightEncoder.reset();

    // Start the motors
    if(distance > 0) {
      setPower(AUTO_STRAIGHT_POWER,AUTO_STRAIGHT_POWER);
    }
    else {
      setPower(-AUTO_STRAIGHT_POWER,-AUTO_STRAIGHT_POWER);
    }

    // Start the state machine
    m_state = straight;  
  }

  ////////////////////////////////////////////////////////////////////
  // Auto Rotate (in deg, negative means counter-clockwise)
  void autoRotate(int deg) {
    // Immediate stop condition
    if(deg == 0) {
      m_state = idle;
      setPower(0, 0);
      return;
    }

    // Convert degress to distance (based on wheel-base's circle circumference)
    int distance = WHEEL_BASE_MM * PI * (long)deg / 360;

    // Set the target ticks
    m_leftTargetTicks = m_leftEncoder.getNumTicksInDistance(distance);
//    m_rightTargetTicks = m_rightEncoder.getNumTicksInDistance(distance);

    Serial.print("AutoRotate: ");
    Serial.print(deg);
    Serial.print("deg=");
    Serial.print(m_leftTargetTicks);
    Serial.println("ticks");

    // Reset the encoders and set the direction of motion
    m_leftEncoder.reset();
//    m_rightEncoder.reset();

    // Start the motors
    if(distance > 0) {
      setPower(AUTO_TURN_POWER, -AUTO_TURN_POWER);
    }
    else {
      setPower(-AUTO_TURN_POWER, AUTO_TURN_POWER);
    }

    // Start the state machine
    m_state = rotate;  
  }

  ////////////////////////////////////////////////////////////////////
  // Drive until one of the line sensors sees a line
  void autoDriveToLine() {
    setPower(LINE_FOLLOW_STRAIGHT_POWER, LINE_FOLLOW_STRAIGHT_POWER);
    m_state = driveToLine;
  }

  ////////////////////////////////////////////////////////////////////
  // Follow a black line.  This should be called as often as possible.
  // Note:  This auto doesn't follow the same structure as the others.
  void autoLineFollow() {
    static bool lastTurnLeft = false;
    
    // Check if all three sensors see black (perpendicular to a black line)
    if((digitalRead(LINE_LEFT_PIN) == 0) && 
       (digitalRead(LINE_MIDDLE_PIN) == 0) && 
       (digitalRead(LINE_RIGHT_PIN) == 0)) {
      setPower(0, 0);  
    }
    if(digitalRead(LINE_MIDDLE_PIN) == 0) {
      // Black line is in the middle, keep going
      setPower(LINE_FOLLOW_STRAIGHT_POWER, LINE_FOLLOW_STRAIGHT_POWER);
    }
    else if(digitalRead(LINE_LEFT_PIN) == 0) {
      // Black line is under the left sensor to go left to bring it to the middle
      setPower(-LINE_FOLLOW_TURN_POWER, LINE_FOLLOW_TURN_POWER);
      lastTurnLeft = true;
    }
    else if(digitalRead(LINE_RIGHT_PIN) == 0) {
      // Black line is under the right sensor to go right to bring it to the middle
      setPower(LINE_FOLLOW_TURN_POWER, -LINE_FOLLOW_TURN_POWER);
      lastTurnLeft = false;
    }
    else {
      // Probably overshot the line, turn hard towards where we were turning before
      if(lastTurnLeft) {
        setPower(-LINE_FOLLOW_TURN_POWER, LINE_FOLLOW_TURN_POWER);
      }
      else {
        setPower(LINE_FOLLOW_TURN_POWER, -LINE_FOLLOW_TURN_POWER);
      }
    }
  }
};

#endif
