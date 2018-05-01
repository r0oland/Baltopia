#include <Arduino.h>
#include "..\lib\Joe\slave_lib.h"

void receive(int numBytes) {};

//create object
EasyTransferI2C ET;

// data struct can't be larger than 22 bytes
// (26 bytes is max for i2c and we have 4 bytes protocol overhead)
struct DataStruct{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int8_t i8 = 0;
  uint8_t ui8 = 0;
  int16_t i16 = 0;
};

//give a name to the group of data
DataStruct mydata;
DataStruct sendData;
DataStruct sendData2;
const uint8_t PROTOCOL_OVERHEAD = 4; // 4 bytes, 0x06, 0x85, size and checksum
//define slave i2c address
const uint8_t I2C_SLAVE_ADDRESS = 9;
const uint8_t I2C_SLAVE_2_ADDRESS = 10;
const uint8_t MAX_I2C_STRUCT_SIZE = 26;

const int sclPin = D1;
const int sdaPin = D2;

void setup(){
  // check that struct is not to large!
  uint8_t structSize = sizeof(mydata) +  PROTOCOL_OVERHEAD;
  if (structSize > MAX_I2C_STRUCT_SIZE){
    while(1) {
      // this is a failsafe, do not remove! change your struct size!
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

  Wire.begin(sdaPin, sclPin);
  Wire.onReceive(receive);
  //start the library, pass in the data details and the name of the serial port
  ET.begin((byte*)&mydata, sizeof(mydata), &Wire);
  //same as ET.begin(details(mydata), &Wire);
  Serial.begin(9600);
  Serial.println("===========================================================");
  Serial.println("Master ready to go!");
}

void loop(){
  //this is how you access the variables. [name of the group].[variable name]
  mydata.i8 = random(-127,127);
  mydata.ui8 = random(0,255);
  mydata.i16 = random(-32767,32767);

  sendData = mydata;
  //send the data
  Serial.println("===========================================================");
  Serial.println("Sending data to slave 1...");
  ET.sendData(I2C_SLAVE_ADDRESS);
  delay(500);

  uint8_t structSize = sizeof(mydata);
  Serial.println("Requesting from slave ");
  Serial.print(structSize);
  Serial.println(" bytes!");

  Serial.println("waiting for answer...");
  uint8_t waitForAnswer = true;
  uint8_t recievedAnswer = true;
  uint32_t waitTime = 2000;
  uint32_t startTime = millis();
  Wire.requestFrom(I2C_SLAVE_ADDRESS,structSize+PROTOCOL_OVERHEAD);
  // wait for answer
  while(waitForAnswer){
    if ((millis() - startTime) > waitTime){
      recievedAnswer = false; // we are done here!
      break;
    }
    if(ET.receiveData()){
      recievedAnswer = true; // we are done here!
      break;
    }
  }
  // transfer of one byte approx. 15 us, very fast!

  Serial.print("[TX] int8_t: ");
  Serial.println(sendData.i8);
  Serial.print("[TX] uint8_t: ");
  Serial.println(sendData.ui8);
  Serial.print("[TX] int16_t: ");
  Serial.println(sendData.i16);

  // display answer
  if (recievedAnswer) {
    Serial.print("[RX] int8_t: ");
    Serial.println(mydata.i8);
    Serial.print("[RX] uint8_t: ");
    Serial.println(mydata.ui8);
    Serial.print("[RX] int16_t: ");
    Serial.println(mydata.i16);
  }
  else
    Serial.println("[RX] timeout!");


  sendData2 = mydata;
  //send the data
  Serial.println("===========================================================");
  Serial.println("Sending data to slave 2...");
  ET.sendData(I2C_SLAVE_2_ADDRESS);
  delay(500);

  structSize = sizeof(mydata);
  Serial.println("Requesting from slave ");
  Serial.print(structSize);
  Serial.println(" bytes!");

  Serial.println("waiting for answer...");
  waitForAnswer = true;
  recievedAnswer = true;
  waitTime = 2000;
  startTime = millis();
  Wire.requestFrom(I2C_SLAVE_2_ADDRESS,structSize+PROTOCOL_OVERHEAD);
  // wait for answer
  while(waitForAnswer){
    if ((millis() - startTime) > waitTime){
      recievedAnswer = false; // we are done here!
      break;
    }
    if(ET.receiveData()){
      recievedAnswer = true; // we are done here!
      break;
    }
  }

  Serial.print("[TX] int8_t: ");
  Serial.println(sendData.i8);
  Serial.print("[TX] uint8_t: ");
  Serial.println(sendData.ui8);
  Serial.print("[TX] int16_t: ");
  Serial.println(sendData.i16);

  // display answer
  if (recievedAnswer) {
    Serial.print("[RX] int8_t: ");
    Serial.println(mydata.i8);
    Serial.print("[RX] uint8_t: ");
    Serial.println(mydata.ui8);
    Serial.print("[RX] int16_t: ");
    Serial.println(mydata.i16);
  }
  else
    Serial.println("[RX] timeout!");



  delay(2000);
}
