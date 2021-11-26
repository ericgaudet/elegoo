#include "elegoo_car.h"
#include "ElegooDriveMotors.h"

ElegooDriveMotors::ElegooDriveMotors(): elegooCar() {
  // Nothing more to initialize
}

void ElegooDriveMotors::drive(int leftPower, int rightPower) {
  // Need to convert -100..100 range to -255..255 range
  elegooCar.setSpeed( leftPower*255/100, rightPower*255/100 );
}
