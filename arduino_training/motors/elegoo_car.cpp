// Code for driving the Elegoo robot's wheel motors
#include <Arduino.h>
#include "elegoo_car.h"

// Motor pins
const int c_u8ENAPin = 5;
const int c_u8ENBPin = 6;
const int c_u8IN1Pin = 7;
const int c_u8IN2Pin = 8;
const int c_u8IN3Pin = 9;
const int c_u8IN4Pin = 11;


ElegooCar::ElegooCar() :
  m_i16PrevLeft( 0 ),
  m_i16PrevRight( 0 )
{
//  // Set Motor Pins to OUTPUT
//  pinMode( c_u8ENAPin, OUTPUT );
//  pinMode( c_u8ENBPin, OUTPUT );
//  pinMode( c_u8IN1Pin, OUTPUT );
//  pinMode( c_u8IN2Pin, OUTPUT );
//  pinMode( c_u8IN3Pin, OUTPUT );
//  pinMode( c_u8IN4Pin, OUTPUT );
//  
//  // Set Line Tracking Pins to Input
//  pinMode( c_u8LineRightPin, INPUT );
//  pinMode( c_u8LineMiddlePin, INPUT );
//  pinMode( c_u8LineLeftPin, INPUT );
}

void ElegooCar::setSpeed( int i16LeftPwr, int i16RightPwr ) {
  if( m_i16PrevLeft != i16LeftPwr ) {
//    Serial.print( "L " ); Serial.println( i16LeftPwr );
    setLeftPwr( i16LeftPwr );
    m_i16PrevLeft = i16LeftPwr;
  }
  if( m_i16PrevRight != i16RightPwr ) {
//    Serial.print( "R " ); Serial.println( i16RightPwr );
    setRightPwr( i16RightPwr );
    m_i16PrevRight = i16RightPwr;    
  }
}

void ElegooCar::setPwr( int s16Pwr, int u8ENAPin, int u8IN1Pin, int u8IN2Pin ) {
  if( s16Pwr > 255 )
    s16Pwr = 255;
  else if( s16Pwr < -255 )
    s16Pwr = -255;
    
  if( s16Pwr > 0 ) {
    digitalWrite( u8IN1Pin, HIGH );
    digitalWrite( u8IN2Pin, LOW );
    analogWrite( u8ENAPin, s16Pwr );
  }
  else if( s16Pwr < 0 ) {
    digitalWrite( u8IN1Pin, LOW );
    digitalWrite( u8IN2Pin, HIGH );
    analogWrite( u8ENAPin, -s16Pwr );
  }
  else {
    digitalWrite( u8IN1Pin, LOW );
    digitalWrite( u8IN2Pin, LOW );
    analogWrite( u8ENAPin, 0 );
  }
}

void ElegooCar::setLeftPwr( int s16Pwr ) {
  setPwr( s16Pwr, c_u8ENAPin, c_u8IN1Pin, c_u8IN2Pin );
}

void ElegooCar::setRightPwr( int s16Pwr ) {
  setPwr( s16Pwr, c_u8ENBPin, c_u8IN4Pin, c_u8IN3Pin );
}
