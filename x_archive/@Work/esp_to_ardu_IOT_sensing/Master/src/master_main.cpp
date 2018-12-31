#include <Arduino.h>
#include "..\lib\Joe\master_lib.h"

// Wifi related
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>

// get time over ze interwebs!
int timezone = 3;
int dst = 0;


const int led = 13;
WiFiClient  client;

uint32_t counter = 0;

// Sensor related %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void receive(int numBytes) {};

//create object
EasyTransferI2C ET;

struct SensorStruct{
  uint32_t gypsumResistance = 0;
  float humd = 0.0;
  float temp = 0.0;
};


SensorStruct mySensorData;


uint8_t structSize = sizeof(mySensorData) +  PROTOCOL_OVERHEAD;

void setup(){
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

  Wire.begin(D1, D2);
  Wire.onReceive(receive);
  //start the library, pass in the data details and the name of the serial port
  ET.begin((byte*)&mySensorData, sizeof(mySensorData), &Wire);
  //same as ET.begin(details(mySensorData), &Wire);
  WiFi.begin(SSID, WIFI_PASS);
  Serial.begin(9600);
  ThingSpeak.begin(client);
  Serial.println("");

  Serial.print("Connecting to ");
  Serial.println(SSID);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  // server.begin();
  Serial.println("HTTP server started");

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("");
  Serial.println("===========================================================");
  Serial.println("Huzzah ready to go!");
}

void loop(){
  time_t now = time(nullptr);
  Serial.println(ctime(&now));

  Serial.print("Requesting from slave ");
  Serial.print(structSize);
  Serial.println(" bytes!");

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

  if (recievedAnswer) {
    Serial.print("[RX] Room temperature: ");
    Serial.println(mySensorData.temp);
    Serial.print("[RX] Room Humidity: ");
    Serial.println(mySensorData.humd);
    Serial.print("[RX] Soil sensor resistance: ");
    Serial.println(mySensorData.gypsumResistance);
    // server.handleClient();
    Serial.print("Sending data to thingsspeak...");

    ThingSpeak.setField(1, float(mySensorData.temp));
    ThingSpeak.setField(2, float(mySensorData.humd));
    ThingSpeak.setField(3, float(mySensorData.gypsumResistance));
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    Serial.println("done!");
    delay(60*1000L);
  }
  else
    Serial.println("[RX] timeout!");
}
