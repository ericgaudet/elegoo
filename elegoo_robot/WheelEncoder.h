// Class that manages the wheel encoders
#ifndef WHEELENCODERS_H
#define WHEELENCODERS_H

// Need left and right side items because the interrupts don't work with class functions
int g_leftCount = 0;
int g_rightCount = 0;
bool g_leftDirectionForward = false;
bool g_rightDirectionForward = false;
void leftTickIsr(void) {
  if(g_leftDirectionForward) {
    g_leftCount++;
  }
  else {
    g_leftCount--;
  }
#ifdef DRIVE_ONLY
  Serial.print("L");
  Serial.println(g_leftCount);
#endif
}

void rightTickIsr(void) {
  if(g_rightDirectionForward) {
    g_rightCount++;
  }
  else {
    g_rightCount--;
  }
#ifdef DRIVE_ONLY
  Serial.print("R");
  Serial.println(g_rightCount);
#endif
}


// Poll left encoder
int g_pollCount = 0;
void pollLeftEncoder() {
  static bool lastEncoderState = false;
  if(digitalRead(LEFT_WHEEL_ENCODER_PIN) != lastEncoderState)
  {
    lastEncoderState = !lastEncoderState;
    if(g_leftDirectionForward) {
      g_pollCount++;
    }
    else {
      g_pollCount--;
    }
  }
}
int getPollCount() {
  return g_pollCount;
}


// Now the class
class WheelEncoder {
private:
  int m_encoderPin;
  int *m_pCount;
  bool *m_pForward;
  float m_ticksToMmFactor;

public:
  WheelEncoder() {}


  ////////////////////////////////////////////////////////////////////
  // Initializer (constructor wasn't a good place to do this)
  void init(int encoderPin, bool leftSide) {
    m_encoderPin = encoderPin;
    m_ticksToMmFactor = 1.0;

    // Setup the encoder to interrupt on rising and falling edges
    pinMode(m_encoderPin, INPUT_PULLUP);
    if( leftSide ) {
      m_pCount = &g_leftCount;
      m_pForward = &g_leftDirectionForward;
      attachInterrupt(digitalPinToInterrupt(m_encoderPin), leftTickIsr, CHANGE);
    }
    else {
      m_pCount = &g_rightCount;
      m_pForward = &g_rightDirectionForward;
      attachInterrupt(digitalPinToInterrupt(m_encoderPin), rightTickIsr, CHANGE);
    } 
  }

  /////////////////////////////////////////////////////////////
  // Reset the encoder tick count to 0
  void reset(void) {
    *m_pCount = 0;
    g_pollCount = 0;
    attachInterrupt(digitalPinToInterrupt(m_encoderPin), leftTickIsr, CHANGE);
  }

  /////////////////////////////////////////////////////////////
  // Tell the encoder if ticks should increment or decrement the count
  void setDirectionForward(bool forward) {
    *m_pForward = forward;
  }

  /////////////////////////////////////////////////////////////
  // Sets the ticks to mm factor so we can work in mm instead of ticks
  void setTicksToDistanceFactor(float factor) {
    m_ticksToMmFactor = factor;
  }

  /////////////////////////////////////////////////////////////
  // Get distance in ticks
  int getDistanceInTicks(void) {
    return g_pollCount;//*m_pCount;
  }

  /////////////////////////////////////////////////////////////
  // Get current distance (uses expensive float calculation)
  int getDistanceMm(void) {
    return *m_pCount / m_ticksToMmFactor;
  }

  /////////////////////////////////////////////////////////////
  // Get number of ticks that represents the distance (quicker than getting mm)
  int getNumTicksInDistance(int distanceMm) {
#if 0
    Serial.print("Ticks in Distance: ");
    Serial.print(distanceMm);
    Serial.print("mm = ");
    Serial.print((float)distanceMm * m_ticksToMmFactor);
    Serial.print("(f) or ");
    Serial.print((int)((float)distanceMm * m_ticksToMmFactor));
    Serial.println("(i) ticks");
#endif    
    return (int)((float)distanceMm * m_ticksToMmFactor);
  }
  
};

#endif
