#include <Servo.h>

#define SERVO_PIN 3 // 3 for Elegoo, 10 for UCtronics, 9 for Eric's Elegoo

Servo myServo;

void setup() {
  Serial.begin(115200);         // Open the serial port to the PC
  Serial.println("Servo v1.0"); // Send an I'm-running message
  myServo.attach(SERVO_PIN);    // Connect the servo to the pin
  myServo.write(0);             // Set the position to 0 deg
}

void loop() {
  myServo.write(0);   // Set the position to 0 deg
  delay(1000); 
  myServo.write(45);  // Set the position to 45 deg
  delay(1000);
}