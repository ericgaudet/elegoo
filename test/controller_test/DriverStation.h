#ifndef DRIVERSTATION_H
#define DRIVERSTATION_H

// ToDo:
//  Add Watchdog to ensure loops are not taking too long.
// 
#define DS_DEBUG 0

#define WDOG_MASK( x ) (bWDExpired() ? 0 : (x))

// Data should arrive every 100ms on average...give a little leaway
#define DATA_EXPIRE_TIME    110

enum {
  ePreGame,
  eAutonomous,
  eTeleop,
  ePostGame
};

class DriverStation {
  uint32_t  m_u32StateChangeTime;
  uint32_t  m_u32DataExpireTime;
  union {
    struct GameData {
      uint8_t   u8Preamble;
      uint8_t   u8Version;
      uint8_t   u8Len;
      uint8_t   u8GameState;
      uint16_t  u16Buttons;
      uint8_t   u8LTrig;
      uint8_t   u8RTrig;
      int8_t    i8LX;
      int8_t    i8LY;
      int8_t    i8RX;
      int8_t    i8RY;
      uint8_t   u8User1;
      uint8_t   u8User2;
      uint16_t  u16Sum;
    } gd;
    uint8_t gru8Buff[sizeof(gd)];
  } m_inBuff;
  uint8_t  m_u8Rcvd;
  uint8_t  m_u8GameState;
  uint16_t m_u16Buttons;
  uint8_t  m_u8LTrig;
  uint8_t  m_u8RTrig;
  int8_t   m_i8LX;
  int8_t   m_i8LY;
  int8_t   m_i8RX;
  int8_t   m_i8RY;
  bool     m_bValid;
  bool     m_bSlowSent;
  
#if DS_DEBUG
  char grcHex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
  void ToHex( char *psz, uint8_t v )
  {
    *psz++ = grcHex[ (v >> 4) & 0x0f ];
    *psz++ = grcHex[ v & 0x0f ];
    *psz++ = ' ';
    *psz++ = '\0';
  }
#endif

  void vWatchDogReset() {
    m_u32DataExpireTime = millis() + DATA_EXPIRE_TIME;
    m_bSlowSent = false;
  }
  bool bWDExpired() {
    if( m_bSlowSent || millis() > m_u32DataExpireTime ) { 
      if( !m_bSlowSent ) {
        Serial.println( "Watchdog: Loop took too long" );
        m_bSlowSent = true;
      }
      return true;
    }
    return false;
  }
public:
  DriverStation() :
    m_u32StateChangeTime( millis() ),
    m_u8Rcvd( 0 ),
    m_bValid( false ),
    m_bSlowSent( false ) {
  }
  ~DriverStation() {
  }

  // returns the current GameState :
  //  ePreGame
  //  eAutonomous
  //  eTelop
  //  ePostGame
  uint8_t getGameState() const { return m_u8GameState; }

  uint32_t getStateTimer() const { return millis() - m_u32StateChangeTime; }
  
  // functions to get the analog values from the game controller
  // in ePreGame, eAutonomous, and eTelop, these functions will return 0
  // returned Left/Right X/Y values are -256 - +255 (actually -254 - +254)
  int16_t getLX() { return WDOG_MASK(m_i8LX * 2); }
  int16_t getLY() { return WDOG_MASK(m_i8LY * 2); }
  int16_t getRX() { return WDOG_MASK(m_i8RX * 2); }
  int16_t getRY() { return WDOG_MASK(m_i8RY * 2); }
  // trigger values are 0 - 255
  uint8_t getLTrig() { return WDOG_MASK(m_u8LTrig); }
  uint8_t getRTrig() { return WDOG_MASK(m_u8RTrig); }
  
  // Request button state
  bool getButton( uint8_t buttonId ) {
    if( buttonId >=16 ) {
      Serial.print( "DriverStation::getButton(): Invalid Button ID requested " ); Serial.println( buttonId );
      return false;
    }
    return WDOG_MASK((m_u16Buttons & (1 << buttonId)) ? true : false);
  }
  
  // Update function must be called repeatedly to read control data from
  // the DriverStation application.
  // When this function returns true, it indicates that new controller
  // data is available.
  bool bUpdate() {
    while( Serial.available() ) {
      int c = Serial.read();
  
      if( m_u8Rcvd == 0 ) {
        if( c == 0xA5 )
          m_inBuff.gru8Buff[m_u8Rcvd++] = c & 0xff;
      }
      else if( m_u8Rcvd == 1 ) {
        if( c == 1 ) {
          m_inBuff.gru8Buff[m_u8Rcvd++] = c & 0xff;
        }
        else {
          Serial.println( "DriverStation: Incorrect Version" );
          m_u8Rcvd = 0;
        }
      }
      else if( m_u8Rcvd == 2 ) {
        if( c == sizeof(m_inBuff.gd) ) {
          m_inBuff.gru8Buff[m_u8Rcvd++] = c & 0xff;
        }
        else {
          Serial.println( "DriverStation: Incorrect Size" );
          m_u8Rcvd = 0;
        }
      }
      else if( m_u8Rcvd < sizeof(m_inBuff.gd) )
      {
        m_inBuff.gru8Buff[m_u8Rcvd++] = c & 0xff;
        if( m_u8Rcvd == sizeof(m_inBuff.gd) ) {
          m_u8Rcvd = 0;   // get ready for next packet

          uint16_t sum = 0;
          for( uint8_t i = 0; i < sizeof(m_inBuff.gd) - 2; i++ ) {
            sum += m_inBuff.gru8Buff[i];
          }

          if( sum == m_inBuff.gd.u16Sum ) {
            // if the game state has changed, remember when it happend
            if( m_u8GameState != m_inBuff.gd.u8GameState )
              m_u32StateChangeTime = millis();

            // update the DriverStation info
            m_u8GameState = m_inBuff.gd.u8GameState;
            m_u16Buttons = m_inBuff.gd.u16Buttons;
            m_u8LTrig = m_inBuff.gd.u8LTrig;
            m_u8RTrig = m_inBuff.gd.u8RTrig;
            m_i8LX = m_inBuff.gd.i8LX;
            m_i8LY = m_inBuff.gd.i8LY;
            m_i8RX = m_inBuff.gd.i8RX;
            m_i8RY = m_inBuff.gd.i8RY;

            vWatchDogReset();
            return true;
         }
          else {
            Serial.print( "Incorrect Checksum : " ); Serial.println( sum, HEX );
#if DS_DEBUG
            for( uint8_t i = 0; i < sizeof(inBuff); i++ ) {
              ToHex( szTmp, inBuff.gru8Buff[i] );
              delay( 10 );
              Serial.print( '.' );
            }
            Serial.println();
#endif
          }
        }
      }
    }
    return false;
  }
};
#endif