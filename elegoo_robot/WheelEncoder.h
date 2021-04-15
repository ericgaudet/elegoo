

// Simple wheel encoder functionality
// 

class WheelEncoder {
private:
  int m_encoderPin;
  int m_count;
  bool m_forward;
  float m_ticksToMmFactor;

public:
  WheelEncoder(int encoderPin) {
    m_encoderPin = encoderPin;
    m_count = 0;
    m_forward = true;
    m_ticksToMmFactor = 1;

    // Setup the encoder to interrupt on rising and falling edges
    pinMode(m_encoderPin, INPUT);
//    attachInterrupt(digitalPinToInterrupt(m_encoderPin), tickIsr1, CHANGE);
  }

  /////////////////////////////////////////////////////////////
  // Reset the encoder tick count to 0
  void reset(void) {
    m_count = 0;
  }

  /////////////////////////////////////////////////////////////
  // Tell the encoder if ticks should increment or decrement the count
  void setDirectionForward(bool forward) {
    m_forward = forward;
  }

  /////////////////////////////////////////////////////////////
  // Sets the ticks to mm factor so we can work in mm instead of ticks
  void setTicksToDistanceFactor(float factor) {
    m_ticksToMmFactor = factor;
  }

  /////////////////////////////////////////////////////////////
  // Register an encoder tick
  void tick(void) {
    if(m_forward) {
      m_count++;
    }
    else {
      m_count--;
    }
  }

  /////////////////////////////////////////////////////////////
  // Get distance in ticks
  int getDistanceInTicks(void) {
    return m_count;
  }

  /////////////////////////////////////////////////////////////
  // Get current distance (uses expensive float calculation)
  int getDistanceMm(void) {
    return m_count / m_ticksToMmFactor;
  }

  /////////////////////////////////////////////////////////////
  // Get number of ticks that represents the distance (quicker than getting mm)
  int getNumTicksInDistance(int distanceMm) {
    return distanceMm * m_ticksToMmFactor;
  }
  
};
