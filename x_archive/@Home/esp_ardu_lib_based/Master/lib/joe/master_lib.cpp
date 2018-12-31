#include "Arduino.h"
#include "master_lib.h"

// I2C functions %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void checkStructSize(HardwareSerial &Serial, uint8_t structSize)
{
  // check that struct is not to large!
  structSize = structSize +  PROTOCOL_OVERHEAD; // add overhead
  Serial.print("I2C data struct size:");
  Serial.print(structSize);
  Serial.println("bytes.");
  if (structSize > MAX_I2C_STRUCT_SIZE)
  {
    while(1)  // this is a failsafe, do not remove! change your struct size!
    {
      Serial.println("WARNING - STRUCT SIZE TO LARGE!");
      Serial.println("THIS WILL CRASH THE ESP8266!");
      Serial.print("Data struct size (struct + overhead) is ");
      Serial.print(structSize);
      Serial.println(" bytes!");
      Serial.print("Maximum allowed struct size is ");
      Serial.print(MAX_I2C_STRUCT_SIZE);
      Serial.println(" bytes!");
      delay(2000);
    }
  }
}
