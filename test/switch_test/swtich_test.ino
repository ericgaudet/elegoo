// Simple switch which can be wired to C and N.O. or C and N.C.
//  - N.O. = Normally open (current only flows when the switch is pressed)
//  - N.C. = Normally closed (current stops flowing when the switch is pressed)
//
// In this example, the switch is wired like this:
//    - A0 > Switch C
//    - Switch N.O > GND

#define SWITCH_INPUT_PIN  A0

void setup() {
  Serial.begin(115200);
  Serial.println("Switch Test v1");

  // Since the switch isn't connected to power, use the microcontroller's
  // internal pull-up resistor to read a 1 when the switch isn't grounded.
  pinMode(SWITCH_INPUT_PIN, INPUT_PULLUP);
}

void loop() {
  // We should see the switch read as 1 when not pressed and 0 when pressed
  // Connect to N.C. instead of N.O. if you want to see the reverse
  Serial.println(digitalRead(SWITCH_INPUT_PIN));
  delay(200);
}
