//#include "UCMotor.h"
//#include "UCNEC.h"

class UctronicsDriveMotors {
 public:
  UctronicsDriveMotors();
  // Power is 0..100 for forward and 0..-100 for reverse
  void drive(int leftPower, int rightPower);

 private:
//  UC_DCMotor leftMotor1;
//  UC_DCMotor rightMotor1;
};
