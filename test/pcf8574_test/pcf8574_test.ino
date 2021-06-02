// The PCF8574 I/O expander board adds 8 input/output pins to your Arduino
// It consumes the two I2C pins (to talk to it) so you gain a net of 6 pins.
// Each pin can be used as either an input or an output.

// Include the wire library to get I2C bus functionality (similar to Serial).
#include <Wire.h>

// The PCF8574's default address is 0x20 but you can modify that address using
// the 3 yellow jumpers on the board.  This gives you an address range or
// 0x20 to 0x27.  Changing the address is only necessary when you have multiple
// I2C devices on the same bus with the same address (which is not allowed).
#define PCF8574_I2C_ADDRESS         0x20

// Define the pins to make them a bit easier to use
#define P0  0x01
#define P1  0x02
#define P2  0x04
#define P3  0x08
#define P4  0x10
#define P5  0x20
#define P6  0x40
#define P7  0x80

// By default, we set all the pins to ones so they can be used as inputs
#define PCF8574_DEFAULT_PIN_VALUES  (P7 | P6 | P5 | P4 | P3 | P2 | P1 | P0)


// Set the pins on the I/O expander board
int setExpanderPins(unsigned char pinValues) {
  int status;
  
  Wire.beginTransmission(PCF8574_I2C_ADDRESS);
  Wire.write(pinValues);
  status = Wire.endTransmission();

  return status;
}

// Get the input pin values on the I/O expander board
unsigned char getExpanderPins() {
  unsigned char pinValues;
  
  Wire.requestFrom(PCF8574_I2C_ADDRESS, 1);
  while(Wire.available()) {
    // Should only get one byte
    pinValues = Wire.read();
  }

  return pinValues;
}

void setup() {
  int status = 0;
  
  Serial.begin(115200);
  Serial.println("PCF8574 IO Expander Test v1");

  // Open I2C bus and start/stop a transmission to see if we can talk
  // to the I/O expander board
  // Notes:
  //   - Bits are used to set the value of each of the 8 ports
  //   - For an output, 0 = low and 1 = high
  //   - For an input, first write a 1 to that bit and then you'll be able
  //     to read it's value (0 or 1).  No need to write a 1 each time you read
  //   - All the pins are written to or read at once (one byte)
  //   - In the byte, P0 is the least-significant-bit and P7 is the 
  //     most-significant-bit so in the 8-bit byte, the pins map out to: 
  //             P7 P6 P5 P4 P3 P2 P1 P0
  //   - It helps if you know how to convert from binary to hexadecimal numbers
  //      e.g. To write set P6, P5, P4 and P0 outputs high and all other outputs low
  //                01110001 binary = 0x71 hexidecimal, so write 0x71
  //   - Alternatively, we can define the bit values and "or" them together using '|'
  Wire.begin();
  setExpanderPins(PCF8574_DEFAULT_PIN_VALUES);

  // Print the status
  //   0:success
  //   1:data too long to fit in transmit buffer
  //   2:received NACK on transmit of address
  //   3:received NACK on transmit of data
  //   4:other error
  Serial.println(status);
}

void loop() {
  unsigned char pinValues = 0;
  
  // In this example, P0 is an LED and P1 is a switch.
  // Wiring:
  // - P0 > LED > resistor > GND
  // - P1 > switch > resistor > GND
  
  // When setting the pins, include all outputs you want to set high
  // and all inputs to make sure they stay set to 'input'
  setExpanderPins(P0 | P1);   // LED on
  pinValues = getExpanderPins();
  Serial.println(pinValues, HEX);
  delay(500);

  setExpanderPins(P1);        // LED off
  pinValues = getExpanderPins();
  Serial.println(pinValues, HEX);
  delay(500);
}
