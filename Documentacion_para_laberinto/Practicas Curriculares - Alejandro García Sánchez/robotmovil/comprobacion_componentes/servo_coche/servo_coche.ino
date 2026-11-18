#include <Servo_megaTinyCore.h>

#define IO 14

Servo miServo;

void setup() {
 miServo.attach(IO);
}

void loop() {
  miServo.write(0);
  delay(1000);

  miServo.write(20);
  delay(1000);
}