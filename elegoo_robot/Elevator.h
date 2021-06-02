#include <Servo.h>

#define SERVO_PIN           10
#define UPPER_LIMIT_SWITCH  4
#define LOWER_LIMIT_SWITCH  A3

class Elevator {
private:
  Servo raiseLowerServo;
public:
  Elevator() {
    pinMode(LOWER_LIMIT_SWITCH, INPUT_PULLUP);
    pinMode(UPPER_LIMIT_SWITCH, INPUT_PULLUP);
    raiseLowerServo.attach(SERVO_PIN);
    raiseLowerServo.write(90);
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
      servoPower = power * 90;
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
    return digitalRead(LOWER_LIMIT_SWITCH);
  }

  ////////////////////////////////////////////////////////////////////
  // Returns true of elevator is at the upper limit
  bool isAtUpperLimit() {
    // Switch is wired to read 1 when elevator is at the limit
    return digitalRead(UPPER_LIMIT_SWITCH);
  }
};
