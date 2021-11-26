#include "elegoo_car.h"

class ElegooDriveMotors {
 public:
  ElegooDriveMotors();
  // Power is 0..100 for forward and 0..-100 for reverse
  void drive(int leftPower, int rightPower);

 private:
  ElegooCar elegooCar;
};
