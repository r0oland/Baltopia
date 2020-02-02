#include <Arduino.h>

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(0, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(2, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(2000);
  digitalWrite(0, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(2, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);
}
//
