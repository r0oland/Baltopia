#include <Arduino.h>
#include "..\lib\Joe\slave_lib.h"

// Sensor Reading %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#include <math.h>
#include "SparkFunHTU21D.h"
HTU21D myHumidity;


const uint8_t A_PIN_MOIST_1 = 0;
const uint8_t D_PIN_MOIST_1 = 2;

const uint8_t A_PIN_MOIST_2 = 1;
const uint8_t D_PIN_MOIST_2 = 3;

const float ARDU_HIGH_VOLTAGE = 5;
const float DIODE_VOLTAGE = 0.8;

const uint32_t MOIST_COMPARE_RESISTOR = 12000; // resitance of resistor used in voltage divider

uint8_t nReads = 10; // average 10 values
uint16_t aRead1 = 0;
uint16_t aRead2 = 0;
uint16_t sensVolt1 = 0;
uint16_t sensVolt2 = 0;
float averageAnalog = 0.0;
float rSense1 = 0.0;
float rSense2 = 0.0;

uint32_t lastMeasTime = 0;
uint32_t measDelay = 2000;

// Arduino to ESP Communication %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void receive(int numBytes) {};
void espWifiRequestEvent();

EasyTransferI2C ET;
struct SensorStruct{
  uint32_t gypsumResistance = 0;
  float humd = 0.0;
  float temp = 0.0;
};

SensorStruct mySensorData;

//define slave i2c addresses
uint8_t answered = false;

////////////////////////////////////////////////////////////////////////////////
void setup(){
  Wire.begin(I2C_SLAVE_ADDRESS);
  //start the library, pass in the data details and the name of the serial port. Can be Serial, Serial1, Serial2, etc.
  ET.begin((byte*)&mySensorData, sizeof(mySensorData), &Wire);
  //define handler function on receiving and request data
  Wire.onReceive(receive);
  Wire.onRequest(espWifiRequestEvent);

  // set digital pins to output
  pinMode(D_PIN_MOIST_1, OUTPUT);
  pinMode(D_PIN_MOIST_2, OUTPUT);

  myHumidity.begin();


  Serial.begin(9600);
  Serial.println("===========================================================");
  Serial.println("Slave ready to go!");
}

void loop() {
  if ((millis() - lastMeasTime) > measDelay){
    lastMeasTime = millis();
    digitalWrite(D_PIN_MOIST_1, HIGH);   // set the voltage supply on
    delay(10);
    aRead1 = (aRead1 + analogRead(A_PIN_MOIST_1))/2; // running average
    digitalWrite(D_PIN_MOIST_1, LOW);    // set the voltage supply off
    delay(10);
    digitalWrite(D_PIN_MOIST_2, HIGH);   // set the voltage supply on
    delay(10);
    aRead2 = (aRead2 + analogRead(A_PIN_MOIST_2))/2;
    digitalWrite(D_PIN_MOIST_2, LOW);    // set the voltage supply off

    float sensVolt1 = aRead1*(ARDU_HIGH_VOLTAGE+DIODE_VOLTAGE)/1023.0;
    float sensVolt2 = aRead2*(ARDU_HIGH_VOLTAGE+DIODE_VOLTAGE)/1023.0;
    rSense1 = MOIST_COMPARE_RESISTOR*ARDU_HIGH_VOLTAGE/sensVolt1-MOIST_COMPARE_RESISTOR;
    rSense2 = MOIST_COMPARE_RESISTOR*ARDU_HIGH_VOLTAGE/sensVolt2-MOIST_COMPARE_RESISTOR;

    mySensorData.gypsumResistance = (rSense1+rSense2)/2;
    Serial.print("[Meas] Sensor Resistance: ");
    Serial.println(mySensorData.gypsumResistance);

    mySensorData.humd = myHumidity.readHumidity();
    mySensorData.temp = myHumidity.readTemperature();

    Serial.print("[Meas] Temperature:");
    Serial.print(mySensorData.temp, 1);
    Serial.println("C");
    Serial.print("[Meas] Humidity:");
    Serial.print(mySensorData.humd, 1);
    Serial.println("%");
    Serial.println(" ");
  }

  if(answered){
    answered = false;
    Serial.print("[TX] Master requested data, I obeyed!");
  }
}

void espWifiRequestEvent()
{
  ET.answer();
  answered = true;
}
