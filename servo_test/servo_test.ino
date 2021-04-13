#include <Servo.h>

#define SERVO_PIN 9

Servo myServo;

void setup() {
  Serial.begin( 115200 );
  Serial.println( "Servo Test v1.2" );
  myServo.attach( SERVO_PIN );
  myServo.write(0);
  
}

void loop() {
  myServo.write(0);
  delay(1000); 
  myServo.write(45);
  delay(1000);
}
