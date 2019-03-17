#include <Baltopia.h> // send data stucts, see in shared libs folder
#include "..\lib\Joe\master_lib.h"

uint8_t read_slave_data(uint8_t slaveAdress);
void check_sensor_struct_size();
uint8_t write_thingsspeak_data();


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void setup(){

  Wire.begin(D2,D1); // still need to remeber what these are...
  //start the library, pass in the data details and the name of the serial port
  Wire.onReceive(receive);
  ET.begin((byte*)&NanoSensorData, sizeof(NanoSensorData), &Wire);

  Serial.begin(9600);

  check_sensor_struct_size();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(SECRET_SSID, SECRET_PASS); // Connect to WPA/WPA2 network. Change this line if using open or WEP network

  ThingSpeak.begin(client);

  Serial.println("===========================================================");
  Serial.println("Master ready to go!");
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void loop(){
  uint8_t recievedAnswer = false;
  Serial.println("Requesting data from slave 1");
  recievedAnswer = read_slave_data(I2C_SLAVE_ADDRESS);
  DataSlave1 = NanoSensorData;

  // Serial.println("Requesting data from slave 2");
  // recievedAnswer = read_slave_data(I2C_SLAVE2_ADDRESS);
  // DataSlave2 = NanoSensorData;

  if (recievedAnswer)
  {
    write_thingsspeak_data();
  }

  delay(60*1000); // update once per minute...still overkill but so what...
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void check_sensor_struct_size()
{
  // check that struct is not to large!
  uint8_t structSize = sizeof(NanoSensorData) + PROTOCOL_OVERHEAD;
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
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
uint8_t read_slave_data(uint8_t slaveAdress)
{
  uint8_t structSize = sizeof(NanoSensorData);
  uint8_t recievedAnswer = false;
  uint32_t startTime;
  uint16_t waitTime = 4000;

  Serial.print(structSize);
  Serial.println(" bytes!");
  startTime = millis();
  Wire.requestFrom(slaveAdress,structSize+PROTOCOL_OVERHEAD);
  Serial.println("waiting for answer...");
  // wait for answer
  while(true){
    if ((millis() - startTime) > waitTime){
      recievedAnswer = false; // we are done here!
      break;
    }
    if(ET.receiveData()){
      recievedAnswer = true; // we are done here!
      break;
    }
  }

  // display answer
  if (recievedAnswer)
  {
    Serial.println("[RX]:");
    Serial.println(NanoSensorData.status);
    Serial.println(NanoSensorData.soilRes1);
    // Serial.println(NanoSensorData.soilRes2);
    Serial.println(NanoSensorData.temp1*INT_TO_TEMPERATURE);
    // Serial.println(NanoSensorData.temp2);
    recievedAnswer = true;
  }
  else
  {
    Serial.println("[RX] timeout!");
  }
  return recievedAnswer;
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
uint8_t write_thingsspeak_data()
{
  // Write value to Field 1 of a ThingSpeak Channel
  float logOhm = DataSlave1.soilRes1;
  float temperature = DataSlave1.temp1*INT_TO_TEMPERATURE;
  Serial.print("Writing logOhm data to thingspeak...");

  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, logOhm);

  // Write the fields that you've set all at once.
  uint8_t httpCode = ThingSpeak.writeFields(SECRET_CH_ID, SECRET_WRITE_APIKEY);


  // uint8_t httpCode = ThingSpeak.writeField(SECRET_CH_ID, 2, logOhm, SECRET_WRITE_APIKEY);
  if (httpCode == 200)
  {
    Serial.println("channel write successful.");
    return 1; // failed
  }
  else
  {
    Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
    return 0; // failed
  }
  //
  // Serial.print("Writing temperature data to thingspeak...");
  // httpCode = ThingSpeak.writeField(SECRET_CH_ID, 1, temperature, SECRET_WRITE_APIKEY);
  // if (httpCode == 200)
  // {
  //   Serial.println("channel write successful.");
  //   return 1; // success
  // }
  // else
  // {
  //   Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
  //   return 0; // failed
  // }
}



// unsigned long myChannelNumber = SECRET_CH_ID;
// const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
