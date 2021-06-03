// Subsystem for the cup gripper (circular jaws that rotate open and closed)
#include "RobotMap.h"
#include <Servo.h>

// Gripper positions
#define OPENED_POS  0
#define CLOSED_POS  60

class Gripper {
private:
  Servo servo;
public:
  ////////////////////////////////////////////////////////////////////
  // Constructor
  Gripper() {}

  ////////////////////////////////////////////////////////////////////
  // Initializer (constructor wasn't a good place to do this)
  void init() {
    servo.attach(GRIPPER_SERVO_PIN);
    open();
  }

  ////////////////////////////////////////////////////////////////////
  // Open the gripper
  void open() {
    servo.write(OPENED_POS);
  }

  ////////////////////////////////////////////////////////////////////
  // Close the gripper
  void close() {
    servo.write(CLOSED_POS);
  }
};
