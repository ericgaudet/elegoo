#include "DriverStation.h"

// Display all of the controller values to allow users to map physical buttons/axes to logical values from the Driver Station
DriverStation ds;

void setup() {
  Serial.begin( 115200 );
  Serial.println( "Controller Test v1.1" );
}

void autonomous() {

}

void teleop() {
  static int count = 5;
  
  // Print the axes values every 5 times (should be twice per second)
  if( --count != 0 ) {
    return;
  }
  count = 5;
  
  Serial.print(  "LX="); Serial.print(ds.getLX());
  Serial.print(", LY="); Serial.print(ds.getLY());
  Serial.print(", RX="); Serial.print(ds.getRX());
  Serial.print(", RY="); Serial.print(ds.getRY());
  Serial.print(", LT="); Serial.print(ds.getLTrig());
  Serial.print(", RT="); Serial.println(ds.getRTrig());

  // Print the button states
  Serial.print("Buttons=");
  for(int i = 0; i < 16; i++) {
    Serial.print(ds.getButton(i));
  }
  Serial.println("");
}

void loop() {
  if( ds.bUpdate() ) {
    switch( ds.getGameState() ) {
      case ePreGame:
      case ePostGame:
        // During Pre and Post game, the Elegoo should not move!
        break;
      case eAutonomous:
        // Handle Autonomous mode
        autonomous();
        break;
      case eTeleop:
        // Handle telop mode
        teleop();
        break;
    }
  }
}
