#include <Baltopia.h> // send data stucts, see in shared libs folder
#include "..\lib\Joe\master_lib.h"

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
WiFiClient  client;

////////////////////////////////////////////////////////////////////////////////
void setup(){
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

  Wire.begin(D2,D1); // still need to remeber what these are...
  //start the library, pass in the data details and the name of the serial port
  Wire.onReceive(receive);
  ET.begin((byte*)&NanoSensorData, sizeof(NanoSensorData), &Wire);
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network

  ThingSpeak.begin(client);

  Serial.println("===========================================================");
  Serial.println("Master ready to go!");
}

////////////////////////////////////////////////////////////////////////////////
void loop(){

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      // WiFi.begin(ssid); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(1000);
    }
    Serial.println("success!");
    Serial.print("IP Address is: ");
    Serial.println(WiFi.localIP());
  }

  //this is how you access the variables. [name of the group].[variable name]
  uint8_t structSize = sizeof(NanoSensorData);
  uint8_t recievedAnswer;
  uint32_t waitTime = 4000;
  uint32_t startTime;

  Serial.println("Requesting data from slave ");
  Serial.print(structSize);
  Serial.println(" bytes!");
  recievedAnswer = false;
  startTime = millis();
  Wire.requestFrom(I2C_SLAVE_ADDRESS,structSize+PROTOCOL_OVERHEAD);
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
  if (recievedAnswer) {
    Serial.print("[RX]  soilRes: ");
    Serial.println(NanoSensorData.soilRes);
    Serial.println(NanoSensorData.status);
    // Write value to Field 1 of a ThingSpeak Channel
    float kOhm = NanoSensorData.soilRes;
    int httpCode = ThingSpeak.writeField(myChannelNumber, 2, kOhm, myWriteAPIKey);

    if (httpCode == 200) {
      Serial.println("Channel write successful.");
    }
    else {
      Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
    }
  }
  else
  {
    Serial.println("[RX] timeout!");
  }
  delay(20000);
}
