class ElegooDriveMotors {
public:
  ElegooDriveMotors();
  // Power is 0..100 for forward and 0..-100 for reverse
  void drive(int leftPower, int rightPower);
};

class UCtronicsDriveMotors {
 public:
  UCtronicsDriveMotors();
  // Power is 0..100 for forward and 0..-100 for reverse
  void drive(int leftPower, int rightPower);
};
