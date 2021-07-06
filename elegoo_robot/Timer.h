#ifndef MYTIMER_H
#define MYTIMER_H

class Timer {
private:
  unsigned long targetTime;
public:
  ////////////////////////////////////////////////////////////////////
  // Constructor
  Timer() {
    targetTime = 0;
  }

  ////////////////////////////////////////////////////////////////////
  // Constructor
  void set(unsigned long timeMs) {
    targetTime = millis() + timeMs;
  }

  ////////////////////////////////////////////////////////////////////
  // Returns true if the current time is >= to the time the timer was set for
  bool isExpired() {
    return (millis() >= targetTime) ? true : false;
  }
};

#endif
