// Sample code for the KY-040 Rotary Encoder
// Polling method (read the encoder signal faster than it can change)

// To test, run the code and turn and push the encoder shaft.

#define ROTARY_ENC_CLK    A0  // Turn the knob
#define ROTARY_ENC_DATA   A1
#define ROTARY_ENC_SWITCH A2  // Push on the knob

bool clockwise = true;
int encoderTicks = 0;
int lastClkState = 0;
bool lastSwitchState = false;

void setup() {
  Serial.begin( 115200 );
  Serial.println( "Rotary Encoder Test v1.0" );
  pinMode(ROTARY_ENC_CLK, INPUT);
  pinMode(ROTARY_ENC_DATA, INPUT);
  pinMode(ROTARY_ENC_SWITCH, INPUT_PULLUP); // Need pull-up because switch is connected to ground

  lastClkState = digitalRead(ROTARY_ENC_CLK);
}

void loop() {
  // Start by assuming we don't have anything new to report
  bool newInformation = false;
  
  // Check if the encoder has moved
  int curClkState = digitalRead(ROTARY_ENC_CLK);
  if(curClkState != lastClkState) {
    // Encoder moved
    newInformation = true;
    
    // Check which direction
    if(digitalRead(ROTARY_ENC_DATA) != curClkState) {
      // CLK pin changed first so we're turning clockwise
      encoderTicks++;
      clockwise = true;
    }
    else {
      encoderTicks--;
      clockwise = false;
    }
    
    lastClkState = curClkState;   
  }

  // Check the switch status
  bool curSwitchState = digitalRead(ROTARY_ENC_SWITCH);
  if(curSwitchState != lastSwitchState) {
    newInformation = true;
    lastSwitchState = curSwitchState;
  }

  if(newInformation) {
    // Print the encoder ticks, direction (+ = clockwise) and switch state
    Serial.print(encoderTicks);
    if(clockwise) {
      Serial.print(" -> Sw=");
    }
    else {
      Serial.print(" <- Sw=");
    }
    Serial.println(lastSwitchState);
  }
}
