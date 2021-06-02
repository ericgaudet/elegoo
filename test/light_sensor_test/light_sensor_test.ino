// The light sensors are typically used to follow a black line on a white background.
//
// How it works:
// - The sensors shining an infrared light at the ground.
// - If the sensors see enough reflection, they report a 1
// - If they don't, they report a 0
// - Each sensor is also equiped with a red LED that turn on when the sensor reports 0
// - The threshold (sensativity) can be set by adjusting the white-and-blue 
//   potentiometer on the light-sensors board.

#define LIGHT_SENSOR_LEFT_PIN   A0  // Usually 2
#define LIGHT_SENSOR_MIDDLE_PIN A1  // Usually 4
#define LIGHT_SENSOR_RIGHT_PIN  A2  // Usually 10


void setup() {
  Serial.begin(115200);
  Serial.println("Line Follower (line sensors) Test v1");

  pinMode(LIGHT_SENSOR_LEFT_PIN, INPUT);
  pinMode(LIGHT_SENSOR_MIDDLE_PIN, INPUT);
  pinMode(LIGHT_SENSOR_RIGHT_PIN, INPUT);
}

void loop() {
  // Simply print the value of each sensor
  // This can be used to adjust the sensativity
  Serial.print(digitalRead(LIGHT_SENSOR_LEFT_PIN));
  Serial.print(digitalRead(LIGHT_SENSOR_MIDDLE_PIN));
  Serial.println(digitalRead(LIGHT_SENSOR_RIGHT_PIN));
  delay(200);
}
