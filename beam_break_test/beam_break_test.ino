// Sample code for the Roarkit Beam-break Sensor used as a switch

// To test, put something between the sensor's emitter and receiver 
// (i.e. put something opaque in the middle of the black U.)

#define ENCODER_PIN 3

void setup() {
  Serial.begin( 115200 );
  Serial.println( "Beam-break Sensor Test v1.0" );
  pinMode(ENCODER_PIN, INPUT);
}

void loop() {
  // Just print the status of the sensor 10 times per second
  Serial.println(digitalRead(ENCODER_PIN));
  delay(100);
}
