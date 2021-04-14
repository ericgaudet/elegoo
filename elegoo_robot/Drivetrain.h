#include "TankDriveSide.h"

// Motor pins
#define L298_ENA_PIN  5
#define L298_ENB_PIN  6
#define L298_IN1_PIN  7
#define L298_IN2_PIN  8
#define L298_IN3_PIN  12  // Was 9
#define L298_IN4_PIN  11

// Line tracking pins
#define LINE_RIGHT_PIN  A2  // Was 10
#define L298_MIDDLE_PIN A1  // Was 4
#define L298_LEFT_PIN   A0  // Was 2


class Drivetrain {
private:
  class TankDriveSide m_leftSide;
  class TankDriveSide m_rightSide;
  
public:
  // Constructor
  Drivetrain(): 
    m_leftSide(L298_ENA_PIN, L298_IN1_PIN, L298_IN2_PIN),
    m_rightSide(L298_ENB_PIN, L298_IN4_PIN, L298_IN3_PIN) {}
    
  // Set left and right side power (0..255)
  void setPower(int left, int right) {
    m_leftSide.setPower(left);
    m_rightSide.setPower(right);
  }
};
