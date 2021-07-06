// Class to control one side (2 motors) of the tank drive
#ifndef TANKDRIVESIDE_H
#define TANKDRIVESIDE_H

class TankDriveSide {
private:
  int m_enPin;
  int m_in1Pin;
  int m_in2Pin;
  int m_curPower;

public:
  // Constructor
  TankDriveSide() {}

  ////////////////////////////////////////////////////////////////////
  // Initializer (constructor wasn't a good place to do this)
  void init(int enPin, int in1Pin, int in2Pin) {
    m_enPin = enPin;
    m_in1Pin = in1Pin;
    m_in2Pin = in2Pin;

    pinMode(m_enPin, OUTPUT);
    pinMode(m_in1Pin, OUTPUT);
    pinMode(m_in2Pin, OUTPUT);

    // Make sure motors are stopped
    m_curPower = 0;
    digitalWrite(m_in1Pin, LOW);
    digitalWrite(m_in2Pin, LOW);
    analogWrite(m_enPin, m_curPower); 
  }

  // Set the motors on this side to specified power (0..255)
  void setPower(int power) {
    // Impose range limit
    if(power > 255) {
      power = 255;
    }
    else if(power < -255) {
      power = -255;
    }

    // Only set the power if it changed
    if(power != m_curPower) {
      // Set the motors to the desired power
      if(power > 0) {
        digitalWrite(m_in1Pin, HIGH);
        digitalWrite(m_in2Pin, LOW);
        analogWrite(m_enPin, power);
      }
      else if(power < 0) {
        digitalWrite(m_in1Pin, LOW);
        digitalWrite(m_in2Pin, HIGH);
        analogWrite(m_enPin, -power);
      }
      else {
        digitalWrite(m_in1Pin, LOW);
        digitalWrite(m_in2Pin, LOW);
        analogWrite(m_enPin, 0);
      }
      m_curPower = power;
    }
  }
};

#endif
