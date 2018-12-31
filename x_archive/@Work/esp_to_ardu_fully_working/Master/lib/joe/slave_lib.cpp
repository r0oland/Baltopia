#include "Arduino.h"
#include "slave_lib.h"

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void check_light_button_pushed() {
  static uint8_t lastButtonState = LOW;
  static uint8_t buttonState = LOW;
  static uint64_t lastDebounceTime = 0;
  uint8_t debounceDelay = 100;

  uint8_t reading = digitalRead(LIGHT_BUTTON_PIN);

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // if the button state has changed:
    if (reading != buttonState){
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        digitalWrite(LIGHT_RELAIS_PIN, !digitalRead(LIGHT_RELAIS_PIN));
      }
    }
  }

  // toggle the light

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
}

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
