#include <Baltopia.h> // send data stucts, see in shared libs folder
#include "..\lib\Joe\slave_lib.h"

DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
void printAddress(DeviceAddress deviceAddress);

////////////////////////////////////////////////////////////////////////////////
void setup(){
  Wire.begin(I2C_SLAVE2_ADDRESS); //I2C: 4 (SDA) and 5 (SCL)
  // start the library, pass in the data details and the name of the serial port.
  // Can be Serial, Serial1, Serial2, etc.
  ET.begin((byte*)&NanoSensorData, sizeof(NanoSensorData), &Wire);
  //define handler function on receiving and request data
  Wire.onReceive(receive);
  Wire.onRequest(data_request_from_master);

  Serial.begin(SERIAL_SPEED);
  Serial.println("===========================================================");
  Serial.println("[HUMID SLAVE] Ready to go!");

  Sensors.begin();
}

////////////////////////////////////////////////////////////////////////////////
void loop() {
  delay(2000);

  Sensors.requestTemperatures();

  Sensors.getAddress(tempDeviceAddress, 0);
    // get address, only works this way when one temperature sensor is connected
  Serial.print("address: ");
	printAddress(tempDeviceAddress);

  Serial.print("Temperature is: ");
  // returns temperature raw value (12 bit integer of 1/128 degrees C)
  NanoSensorData.temp1 = Sensors.getTemp(tempDeviceAddress);
  Serial.print("= is: ");
  Serial.println(NanoSensorData.temp1);


  if(answeredMasterRequest)
  {
    Serial.println("[SLAVE] Send data to master!");
    // Serial.print("[SLAVE] Measured resistance: ");
    // Serial.println(NanoSensorData.soilRes);
    // Serial.println(NanoSensorData.soilRes);
    answeredMasterRequest = false;
  }

}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// support functions %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// I2C interrupt ---------------------------------------------------------------
void data_request_from_master()
{
  // DO NOTHING, ABSOLUTELY NOTHING ELSE HERE!
  ET.answer();
  answeredMasterRequest = true;
}

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
