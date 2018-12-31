#include <Arduino.h>
#include "..\lib\Joe\master_lib.h"

//create object
void receive(int numBytes) {}; // needs to be here for some reason...
EasyTransferI2C ET;

//give a name to the group of data
DataStruct mydata;
DataStruct sendData;

void setup(){
  Serial.begin(9600);
  delay(1000);
  Serial.println("");
  Serial.println("");
  Serial.println("===========================================================");
  Serial.println("Setting up master!");

  // setup i2c, this doing bit banging on the ESP boards but whatever works...
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.onReceive(receive); // define interrupt function to read data from ardu
  //start the library, pass in the data details and the name of the serial port
  checkStructSize(Serial,sizeof(mydata));
  ET.begin((byte*)&mydata, sizeof(mydata), &Wire);
  Serial.println("===========================================================");
  Serial.println("Master ready to go!");
}

void loop(){
  //this is how you access the variables. [name of the group].[variable name]
  mydata.i8 = random(-127,127);
  mydata.ui8 = random(0,255);
  // mydata.i16 = random(-32767,32767);

  sendData = mydata;
  //send the data
  Serial.println("===========================================================");
  Serial.println("Sending data to slave 1...");
  ET.sendData(I2C_SLAVE_ADDRESS);
  Serial.println("[TX] Complete!");

  // delay(500);

  uint8_t structSize = sizeof(mydata);
  Serial.println("[RX] Data requested.");
  uint8_t waitForAnswer = true;
  uint8_t recievedAnswer = false;
  uint32_t waitTime = 2000;
  uint32_t startTime = millis();
  Wire.requestFrom(I2C_SLAVE_ADDRESS,structSize+PROTOCOL_OVERHEAD);
  // wait for answer
  while(waitForAnswer){
    if ((millis() - startTime) > waitTime){
      recievedAnswer = false; // we are done here!
      waitForAnswer = false;
    }
    if(ET.receiveData()){
      recievedAnswer = true; // we are done here!
      waitForAnswer = false;
    }
  }
  // transfer of one byte approx. 15 us, very fast!

  // display answer
  if (recievedAnswer) {
    Serial.println("[RX] Data recieved!");
  }
  else{
    Serial.println("[RX] timeout!");
  }
  delay(500);
}
