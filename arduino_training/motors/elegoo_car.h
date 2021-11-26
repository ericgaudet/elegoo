#ifndef _ELEGOO_CAR_H
#define _ELEGOO_CAR_H

class ElegooCar {
 public:
  ElegooCar();  
  void setSpeed( int i16LeftPwr, int i16RightPwr ); // Power range = -255..255
  
 private:
  int m_i16PrevLeft;
  int m_i16PrevRight;
  
  void setPwr( int s16Pwr, int u8ENAPin, int u8IN1Pin, int u8IN2Pin );
  void setLeftPwr( int s16Pwr );  
  void setRightPwr( int s16Pwr );
};

#endif
