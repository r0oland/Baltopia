#include <Arduino.h>
#include "..\lib\Joe\slave_lib.h"

//create object
EasyTransferI2C ET;

//give a name to the group of data
DataStruct mydata;

//define slave i2c address
uint8_t answered = false;
////////////////////////////////////////////////////////////////////////////////
void setup(){
  Wire.begin(I2C_SLAVE_ADDRESS);
  //start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
  ET.begin((byte*)&mydata, sizeof(mydata), &Wire);
  //define handler function on receiving and request data
  Wire.onReceive(receive);
  Wire.onRequest(espWifiRequestEvent);
  Serial.begin(9600);
  Serial.println("===========================================================");
  Serial.println("Slave ready to go!");
}

void loop() {
  //check and see if a data packet has come in.
  if(ET.receiveData()){
    //this is how you access the variables. [name of the group].[variable name]
    //since we have data, we will blink it out.
    //send the data
    Serial.print("[RX] int8_t: ");
    Serial.println(mydata.i8++);
    Serial.print("[RX] uint8_t: ");
    Serial.println(mydata.ui8++);
    Serial.print("[RX] int16_t: ");
    Serial.println(mydata.i16++);
    Serial.println(" ");
  }
  if(answered){
    answered = false;
    Serial.print("[TX] int8_t: ");
    Serial.println(mydata.i8);
    Serial.print("[TX] uint8_t: ");
    Serial.println(mydata.ui8);
    Serial.print("[TX] int16_t: ");
    Serial.println(mydata.i16);
    Serial.println(" ");
  }

}

void espWifiRequestEvent()
{
  answered = true;
  ET.answer();
}
