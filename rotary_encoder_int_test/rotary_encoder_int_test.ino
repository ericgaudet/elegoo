// Sample code for the KY-040 Rotary Encoder
// Interrupt method (trigger a function every time the encoder signal changes)
//
// This example uses a "pin change" interrupt so we don't have to use pin 2 or 3 for "external" interrupts.
// It could also use the simpler "external" interrupt if pin 2 or 3 were available.

// To test, run the code and turn encoder shaft.

#include "PinChangeInt.h" // From https://github.com/GreyGnome/PinChangeInt

#define ROTARY_ENC_CLK    A0  // Turn the knob
#define ROTARY_ENC_DATA   A1
// #define ROTARY_ENC_SWITCH A2  // Not used in this test

bool clockwise = true;
int encoderTicks = 0;
int lastClkState = 0;
bool newInformation = false;

void setup() {
  Serial.begin( 115200 );
  Serial.println( "Interrupt-based Rotary Encoder Test v1.0" );
  pinMode(ROTARY_ENC_CLK, INPUT);
  pinMode(ROTARY_ENC_DATA, INPUT);
  PCintPort::attachInterrupt(ROTARY_ENC_CLK, encoderTicked, CHANGE);

  lastClkState = digitalRead(ROTARY_ENC_CLK);
}

void loop() {
  if(newInformation) {
    newInformation = false;
    
    // Print the encoder ticks, direction (+ = clockwise) and switch state
    Serial.print(encoderTicks);
    if(clockwise) {
      Serial.println(" ->");
    }
    else {
      Serial.println(" <-");
    }
  }
}

// This is the encoder's Interrupt Service Routine (ISR)
// IMPORTANT:  Do as little as possible in an ISR because everything else in your
//             Arduino is stopped while you are in here.
void encoderTicked() {
  // The encoder pin has transitioned from high->low or low->high.
  // Check which direction
  if(digitalRead(ROTARY_ENC_DATA) != digitalRead(ROTARY_ENC_CLK)) {
    // CLK pin changed first so we're turning clockwise
    encoderTicks++;
    clockwise = true;
  }
  else {
    encoderTicks--;
    clockwise = false;
  }
  newInformation = true;
}
