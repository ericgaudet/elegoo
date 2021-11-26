#define ELEGOO  // UCtronics 2-wheels robot --> comment this line, Elegoo 4-wheels robot --> uncomment this line

#ifdef ELEGOO
#include "ElegooDriveMotors.h"
#define ROBOT_TYPE "Elegoo "
ElegooDriveMotors motors;


#else

#include "UctronicsDriveMotors.h"
#define ROBOT_TYPE "Uctronics "
UctronicsDriveMotors motors;
#endif

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.print(ROBOT_TYPE);
  Serial.println("Motors v1.0");
}

void loop() {
  // put your main code here, to run repeatedly:
  // Drive forward
  motors.drive(50, 50);
  delay(1000);

  // Turn right
  motors.drive(50, -50);
  delay(2000);

}
