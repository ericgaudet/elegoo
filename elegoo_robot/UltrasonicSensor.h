#define ULTRASONIC_TRIG A5 
#define ULTRASONIC_ECHO A4

// Useful constants for ultrasonic calculations
#define MAX_DISTANCE 4500 // mm, some sensors are max 4000
#define SPEED_OF_SOUND_MM_PER_US 0.343  // Dry air, 20degC
#define ULTRASONIC_TIMEOUT ((MAX_DISTANCE + 500) * 2 / SPEED_OF_SOUND_MM_PER_US)  // Add 500mm worth of spare time in the timeout

class UltrasonicSensor {
public:
  UltrasonicSensor() {
    pinMode(ULTRASONIC_TRIG, OUTPUT);
    pinMode(ULTRASONIC_ECHO, INPUT);
  }

  int getDistanceMm() {
    // Using the ultrasonic sensor
    // - TRIG must be low for at least 2us, then high for 10us, then put back low.
    // - If an object is detected, ECHO will output a pulse that will stay high for
    //   the same amount of time that it took for the emitted pulse to go out and return.
  
    // Start by triggering the transmission of the 8-pulse 40kHz signal
    digitalWrite(ULTRASONIC_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG, LOW);
  
    // Check to see if an echo was heard
    // Echo time and timeout are in microseconds (us)
    unsigned long echoTime = pulseIn(ULTRASONIC_ECHO, HIGH, ULTRASONIC_TIMEOUT);
    if(echoTime == 0) {
      // No pulse started before the timeout.  Return an error code.
      return -1;
    }
  
    // Calculate the distance
    // - Take the time that it took to hear the echo and divide by 2 since we only want
    //   the time it took to get to the object, not the time to get there and come back.
    // - Multiply the time it took by the speed of sound (340m/s = 340000mm/s = 0.34mm/us).
    return (int)(echoTime * SPEED_OF_SOUND_MM_PER_US / 2);
  }
};
