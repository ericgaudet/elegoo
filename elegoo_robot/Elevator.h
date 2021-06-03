// Elevator subsystem
// Includes continuous servo control for raise-lower functionality and upper and lower limit switches.
#include "RobotMap.h"
#include <Servo.h>

class Elevator {
private:
  Servo raiseLowerServo;
public:
  ////////////////////////////////////////////////////////////////////
  // Constructor
  Elevator() {}

  ////////////////////////////////////////////////////////////////////
  // Initializer (constructor wasn't a good place to do this)
  void init() {
    pinMode(ELEVATOR_LOWER_LIMIT_SWITCH_PIN, INPUT_PULLUP);
    pinMode(ELEVATOR_UPPER_LIMIT_SWITCH_PIN, INPUT_PULLUP);
    raiseLowerServo.attach(ELEVATOR_SERVO_PIN);
    setPower(0);
  }
  
  ////////////////////////////////////////////////////////////////////
  // Set the elevator power (-256 to 256, negative is down)
  void setPower(int power) {
    int servoPower;
    
    // Map values to servo speeds
    // - -256..0 = servo 90 to 180
    // -  0..256 = servor 90 to 0
    if(power < 0) {
      // Lower elevator
      servoPower = -power * 90;
      servoPower >>= 8; // divide by 256
      servoPower = 90 + servoPower;   
    }
    else if(power > 0) {
      // Raise elevator
      servoPower = power * 90;
      servoPower >>= 8; // divide by 256
      servoPower = 90 - servoPower;
    }
    else {
      servoPower = 90;  // Full-stop on continuous servo
    }
    raiseLowerServo.write(servoPower);
  }

  ////////////////////////////////////////////////////////////////////
  // Returns true of elevator is at the lower limit
  bool isAtLowerLimit() {
    // Switch is wired to read 1 when elevator is at the limit
    return digitalRead(ELEVATOR_LOWER_LIMIT_SWITCH_PIN);
  }

  ////////////////////////////////////////////////////////////////////
  // Returns true of elevator is at the upper limit
  bool isAtUpperLimit() {
    // Switch is wired to read 1 when elevator is at the limit
    return digitalRead(ELEVATOR_UPPER_LIMIT_SWITCH_PIN);
  }
};
