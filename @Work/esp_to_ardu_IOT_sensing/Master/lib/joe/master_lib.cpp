#include "Arduino.h"
#include "master_lib.h"

// request data from nano
void requestEvent() {
  Serial.print("Nano sending data: ");
  uint16_t bigNum = random(0,65535);
  byte myArray[2];
  myArray[0] = (bigNum >> 8) & 0xFF;
  myArray[1] = bigNum & 0xFF;
  Wire.write(myArray,2);
  Serial.println(bigNum);
}

// function that executes whenever data is received from master
void receiveEvent(int numBytes)
{
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    Serial.print(c);         // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  Serial.println(x);         // print the integer
}
