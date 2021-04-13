// Sample code for the Roarkit Beam-break Sensor used as a wheel encoder (with encoder wheels)
// Interrupt method (trigger a function every time the encoder signal changes)
//
// To test, install the encoder and encoder wheel, run the code and manually turn the encoder
// wheel

#define ENCODER_PIN 3

// Keep track of how many ticks the encoder has turned
int encoderTicks = 0;

void setup() {
  Serial.begin( 115200 );
  Serial.println( "Interrupt-based Encoder Test v1.0" );
  pinMode(ENCODER_PIN, INPUT);

  // Tell the Arduino to generate an interrupt every time the encoder pin changes (high->low or
  // low->high).  When the interrupt occurs, call the encoderTick() function.
  // Note that the digitalPinToInterrupt function converts a pin number (e.g. 3) to an
  // interrupt number (e.g. 1).
  //
  // IMPORTANT:  Only pins 2 and 3 of the Arduino Uno can generate "external interrupts" in 
  // this way.  You can use other pins to do this but then you need to use "pin interrupts"
  // which are a bit harder to use (another topic).  
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), encoderTicked, CHANGE);
}

void loop() {
  static int lastEncoderTicks = 0;  // "Static" makes this variable keep its value across loop iterations
  
  if( encoderTicks != lastEncoderTicks) {
    // The tick count has changed, print the new count
    Serial.println(encoderTicks);
    lastEncoderTicks = encoderTicks;
  }
}

// This is the encoder's Interrupt Service Routine (ISR)
void encoderTicked() {
  // The encoder pin has transitioned from high->low or low->high.
  encoderTicks++;
}
