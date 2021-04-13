// Sample code for the Roarkit Beam-break Sensor used as a wheel encoder (with encoder wheels)
// Polling method (read the encoder signal faster than it can change)
//
// To test, install the encoder and encoder wheel, run the code and manually turn the encoder
// wheel

#define ENCODER_PIN 3

// Keep track of how many ticks the encoder has turned
int encoderTicks = 0;
bool lastEncoderState = false;

void setup() {
  Serial.begin( 115200 );
  Serial.println( "Encoder Test v1.0" );
  pinMode(ENCODER_PIN, INPUT);
}

void loop() {
  // Check if the encoder value has changed
  // Important: The pin must be checked fast enough to not miss a high->low or low->high
  // transition.  If your loop is too slow, you'll need to use interrupts (another topic).
  if(digitalRead(ENCODER_PIN) != lastEncoderState) {
    // Save the changed state by flipping the boolean value (!true is false, !false is true)
    lastEncoderState = !lastEncoderState;

    // Count the number of ticks
    // Note:  We don't know the direction the ticks so we have to figure that out some
    // other way (e.g. when we set the motors forwards or backwards).  For now, assume
    // we're always going in the same direction.
    encoderTicks++;

    // Display the tick count
    Serial.println(encoderTicks);
  }
  
}
