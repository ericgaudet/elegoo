#include "TankDriveSide.h"
#include "WheelEncoder.h"

// Motor pins
#define L298_ENA_PIN  5
#define L298_ENB_PIN  6
#define L298_IN1_PIN  7
#define L298_IN2_PIN  8
#define L298_IN3_PIN  12  // Was 9
#define L298_IN4_PIN  11

// Encoder pins and defines
#define LEFT_ENCODER_PIN    2
#define RIGHT_ENCODER_PIN   3
#define TICKS_TO_MM_FACTOR  (495/1000.0)

// Line tracking pins
#define LINE_RIGHT_PIN  A2  // Was 10
#define L298_MIDDLE_PIN A1  // Was 4
#define L298_LEFT_PIN   A0  // Was 2

enum States {
  idle = 0,
  straight
};

class Drivetrain {
private:
  class TankDriveSide m_leftSide;
  class TankDriveSide m_rightSide;
  class WheelEncoder m_leftEncoder;
  class WheelEncoder m_rightEncoder;
  int m_leftEncoderCount;
  int m_rightEncoderCount;
  int m_leftTargetTicks;
  int m_rightTargetTicks;
  enum States m_state;
  
public:
  // Constructor
  Drivetrain(): 
    m_leftSide(L298_ENA_PIN, L298_IN1_PIN, L298_IN2_PIN),
    m_rightSide(L298_ENB_PIN, L298_IN4_PIN, L298_IN3_PIN),
    m_leftEncoder(LEFT_ENCODER_PIN, true),
    m_rightEncoder(RIGHT_ENCODER_PIN, false) {
      m_leftEncoder.setTicksToDistanceFactor(TICKS_TO_MM_FACTOR);
      m_rightEncoder.setTicksToDistanceFactor(TICKS_TO_MM_FACTOR);
      m_leftTargetTicks = 0;
      m_rightTargetTicks = 0;
      m_state = idle;
  }
    
  // Set left and right side power (0..255)
  void setPower(int left, int right) {
    m_leftSide.setPower(left);
    m_rightSide.setPower(right);
  }

  ////////////////////////////////////////////////////////////////////
  // Tank drive (power -254..254)
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
    switch(m_state) {
    case idle:
      break;
    case straight:
      // Check if we've gotten to the target distance (take into account the direction)
      int curTicks = m_leftEncoder.getDistanceInTicks();
      if((m_leftTargetTicks >= 0 && (curTicks >= m_leftTargetTicks)) ||
         (m_leftTargetTicks < 0 && (curTicks <= m_leftTargetTicks))) {
        setPower(0, 0);
        m_state = idle;
        Serial.print("Ending straight drive (");
        Serial.print(curTicks);
        Serial.print("of ");
        Serial.print(m_leftTargetTicks);
        Serial.println(")");
      }
      break;
    }
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
    m_rightTargetTicks = m_rightEncoder.getNumTicksInDistance(distance);

    // Reset the encoders and set the direction of motion
    m_leftEncoder.reset();
    m_rightEncoder.reset();
    m_leftEncoder.setDirectionForward(distance > 0 ? true : false);
    m_rightEncoder.setDirectionForward(distance < 0 ? true : false);

    // Start the motors
    if(distance > 0) {
      setPower(128,128);
    }
    else {
      setPower(-128,-128);
    }

    // Start the state machine
    m_state = straight;
    
  }
  
};
