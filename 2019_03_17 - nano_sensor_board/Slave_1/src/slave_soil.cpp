<<<<<<< HEAD
#include <Baltopia.h> // send data stucts, see in shared libs folder
#include "..\lib\Joe\slave_lib.h"

#include "OneWire.h" // OneWire by Paul Stoffregen
#include "DallasTemperature.h" // DallasTemperature by Guil Barros
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
void printAddress(DeviceAddress deviceAddress);
const uint8_t ONE_WIRE_BUS = 12; // -127 = no pull down, not connected
OneWire OneWire(ONE_WIRE_BUS);
DallasTemperature Sensors(&OneWire);

#include "Adafruit_Si7021.h"
Adafruit_Si7021 moistSense = Adafruit_Si7021();


////////////////////////////////////////////////////////////////////////////////
void setup(){
  // setup digital output pins
  pinMode(MOIST_SUPPL_1, OUTPUT);
  pinMode(MOIST_SUPPL_2, OUTPUT);
  pinMode(MOIST_SUPPL_3, OUTPUT);
  pinMode(MOIST_SUPPL_4, OUTPUT);

  Wire.begin(I2C_SLAVE_ADDRESS); //I2C: 4 (SDA) and 5 (SCL)
  // start the library, pass in the data details and the name of the serial port.
  // Can be Serial, Serial1, Serial2, etc.
  ET.begin((byte*)&NanoSensorData, sizeof(NanoSensorData), &Wire);
  //define handler function on receiving and request data

  Wire.onReceive(receive);
  Wire.onRequest(data_request_from_master);

  Sensors.begin();

  Serial.begin(SERIAL_SPEED);
  Serial.println("===========================================================");
  Serial.println("[SOIL SLAVE] Ready to go!");


  fullRes1 = measure_resistance(0);
  fullRes2 = measure_resistance(1);
    // measure once to get startign value for movign avarage in loop()

  if (!moistSense.begin())
  {
    Serial.println("Did not find Si7021 moistSense!");
    while (true){};
  }
  else
  {
    Serial.println("Found Si7021 moisture & temperature sensor!");
  }

}

////////////////////////////////////////////////////////////////////////////////
void loop() {
  NanoSensorData.status += 1;

  // Measure plant 1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // measure resistance and convert to better scale (see note below)
  uint32_t currentRes1 = measure_resistance(0);
  fullRes1 = (currentRes1 + fullRes1)/2; // moving average

  // convert full Res to log and multiply, this way we use 16 bit resolution better
  // and linearize the scale a bit...
  NanoSensorData.soilRes1 = 10000*log10(fullRes1);
    // NOTE resistance is in range between 1 and 10^6, so log(10) of resistance
    // is betweem 1 and 6, but we send uint16 integers, with a range of 0-65,535
    // so we take log10 and multiply by 10, thus having a nice, and 'linear'
    // scale with a descent resolution

  // measure one-wire temperature
  Sensors.requestTemperatures();
  Sensors.getAddress(tempDeviceAddress, 0);
  NanoSensorData.temp1 = Sensors.getTemp(tempDeviceAddress);
  float tempCel1 = NanoSensorData.temp1*1.0/128.0; // we send data as int, but display it as float

  // Measure plant 1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  // Measure plant 1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // measure resistance and convert to better scale (see note below)
  uint32_t currentRes2 = measure_resistance(1);
  fullRes2 = (currentRes2 + fullRes2)/2; // moving average

  // convert full Res to log and multiply, this way we use 16 bit resolution better
  // and linearize the scale a bit...
  NanoSensorData.soilRes2 = 10000*log10(fullRes2);
    // NOTE resistance is in range between 1 and 10^6, so log(10) of resistance
    // is betweem 1 and 6, but we send uint16 integers, with a range of 0-65,535
    // so we take log10 and multiply by 10, thus having a nice, and 'linear'
    // scale with a descent resolution

  // measure one-wire temperature
  Sensors.requestTemperatures();
  Sensors.getAddress(tempDeviceAddress, 1);
  NanoSensorData.temp2 = Sensors.getTemp(tempDeviceAddress);
  float tempCel2 = NanoSensorData.temp2*1.0/128.0; // we send data as int, but display it as float

  Serial.print("Humidity:    ");
  Serial.print(moistSense.readHumidity(), 2);
  Serial.print("\tTemperature: ");
  Serial.println(moistSense.readTemperature(), 2);


  delay(1000); // FIXME make once every 10s or so, which is still waay to much...

  // if(answeredMasterRequest || true)
  // {
    Serial.println("----------------------------------------------------------");
    Serial.println("[SLAVE] Send data to master!");
    Serial.print("[SLAVE] Measured resistance 1: ");
    Serial.println(currentRes1);
    Serial.print("[SLAVE] Averaged resistance 1: ");
    Serial.println(fullRes1);
    Serial.print("[SLAVE] Calculated linear resistance 1: ");
    Serial.println(NanoSensorData.soilRes1);
    Serial.print("[SLAVE] Measured temperature 1: ");
    Serial.println(tempCel1);
    Serial.println();
    Serial.print("[SLAVE] Measured resistance 1: ");
    Serial.println(currentRes2);
    Serial.print("[SLAVE] Averaged resistance 1: ");
    Serial.println(fullRes2);
    Serial.print("[SLAVE] Calculated linear resistance 1: ");
    Serial.println(NanoSensorData.soilRes2);
    Serial.print("[SLAVE] Measured temperature 1: ");
    Serial.println(tempCel2);
    Serial.println();
    answeredMasterRequest = false;
  // }

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

void setupCurrentPath(uint8_t iPath, uint8_t changeCurrentPath) {
  // setup current paths for nPaths, where each path is connected to their own
  // moisture sensing gypsum sensor
  switch (iPath)
  {
    case 0: // first current path
      if (doReadForward)
      {
        activeDigitalPin = MOIST_SUPPL_2;
        supplyVoltageAnalogPin = MOIST_SENSE_2;
        sensorVoltageAnalogPin = MOIST_SENSE_1;
      }
      else {
        activeDigitalPin = MOIST_SUPPL_1;
        supplyVoltageAnalogPin = MOIST_SENSE_1;
        sensorVoltageAnalogPin = MOIST_SENSE_2;
      }
      break;
    case 1:
      if (doReadForward)
      {
        activeDigitalPin = MOIST_SUPPL_3;
        supplyVoltageAnalogPin = MOIST_SENSE_3;
        sensorVoltageAnalogPin = MOIST_SENSE_4;
      }
      else {
        activeDigitalPin = MOIST_SUPPL_4;
        supplyVoltageAnalogPin = MOIST_SENSE_4;
        sensorVoltageAnalogPin = MOIST_SENSE_3;
      }
      break;
    default:
      break;
  }

  if (changeCurrentPath)
    doReadForward != doReadForward;
}


int32_t measure_resistance(uint8_t currentPath){
  // Serial.print("[SLAVE] Measuring soil resistance...");
  // http://vanderleevineyard.com/vineyard-blog/-the-vinduino-project-3-make-a-low-cost-soil-moisture-sensor-reader
  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter

  int32_t resistance = 0; // reset resistance
  int32_t tempResistance = 0; // reset resistance
  for (int i=0; i < N_READS; i++)
  {
    setupCurrentPath(currentPath,1);      // Prepare the digital and analog pin values

    digitalWrite(activeDigitalPin, HIGH);                 // set the voltage supply on
    delay(10);
    int16_t supplyReadOut = analogRead(supplyVoltageAnalogPin);   // read the supply voltage
    int16_t sensorReadOut = analogRead(sensorVoltageAnalogPin);   // read the sensor voltage
    digitalWrite(activeDigitalPin, LOW);                  // set the voltage supply off
    delay(10);

    int16_t diffRead = supplyReadOut - sensorReadOut;
      // make sure we don't have short, in which case both values are very close
    if ((sensorReadOut >= 3) & (diffRead>3))
      tempResistance = int32_t(float(COMP_RESISTOR)*(supplyReadOut-sensorReadOut)/sensorReadOut);
    else if ((sensorReadOut >= 3) & !(diffRead>3)) // short, so set res close to zero
      tempResistance = 1; // set to 1M ohm as upper limit
    else
      tempResistance = MEGA*3; // set to 1M ohm as upper limit
        // if sensor readout becomes too small, resistance values become meaningless
        // so set upper limit for possible resistance
    resistance += tempResistance;
  }

  resistance = resistance/N_READS;
  if (resistance > MEGA*3)
    resistance = MEGA*3; // set to mega ohm

  return resistance;
}
=======
#include <Baltopia.h> // send data stucts, see in shared libs folder
#include "..\lib\Joe\slave_lib.h"

#include "OneWire.h" // OneWire by Paul Stoffregen
#include "DallasTemperature.h" // DallasTemperature by Guil Barros
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
void printAddress(DeviceAddress deviceAddress);
const uint8_t ONE_WIRE_BUS = 12; // -127 = no pull down, not connected
OneWire OneWire(ONE_WIRE_BUS);
DallasTemperature Sensors(&OneWire);

#include "Adafruit_Si7021.h"
Adafruit_Si7021 moistSense = Adafruit_Si7021();


////////////////////////////////////////////////////////////////////////////////
void setup(){
  // setup digital output pins
  pinMode(MOIST_SUPPL_1, OUTPUT);
  pinMode(MOIST_SUPPL_2, OUTPUT);
  pinMode(MOIST_SUPPL_3, OUTPUT);
  pinMode(MOIST_SUPPL_4, OUTPUT);

  pinMode(PUMP_PIN_1, OUTPUT);
  pinMode(PUMP_PIN_2, OUTPUT);

  Wire.begin(I2C_SLAVE_ADDRESS); //I2C: 4 (SDA) and 5 (SCL)
  // start the library, pass in the data details and the name of the serial port.
  // Can be Serial, Serial1, Serial2, etc.
  ET.begin((byte*)&NanoSensorData, sizeof(NanoSensorData), &Wire);
  //define handler function on receiving and request data

  Wire.onReceive(receive);
  Wire.onRequest(data_request_from_master);

  Sensors.begin();

  Serial.begin(SERIAL_SPEED);
  Serial.println("===========================================================");
  Serial.println("[SOIL SLAVE] Ready to go!");


  fullRes1 = measure_resistance(0);
  fullRes2 = measure_resistance(1);
    // measure once to get startign value for movign avarage in loop()

  if (!moistSense.begin())
  {
    Serial.println("Did not find Si7021 moistSense!");
    while (true){};
  }
  else
  {
    Serial.println("Found Si7021 moisture & temperature sensor!");
  }

}

////////////////////////////////////////////////////////////////////////////////
void loop() {
  NanoSensorData.status += 1;

  // Measure plant 1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // measure resistance and convert to better scale (see note below)
  uint32_t currentRes1 = measure_resistance(0);
  fullRes1 = (currentRes1 + fullRes1)/2; // moving average

  // convert full Res to log and multiply, this way we use 16 bit resolution better
  // and linearize the scale a bit...
  NanoSensorData.soilRes1 = 10000*log10(fullRes1);
    // NOTE resistance is in range between 1 and 10^6, so log(10) of resistance
    // is betweem 1 and 6, but we send uint16 integers, with a range of 0-65,535
    // so we take log10 and multiply by 10, thus having a nice, and 'linear'
    // scale with a descent resolution

  // measure one-wire temperature
  Sensors.requestTemperatures();
  Sensors.getAddress(tempDeviceAddress, 0);
  NanoSensorData.temp1 = Sensors.getTemp(tempDeviceAddress);
  float tempCel1 = NanoSensorData.temp1*1.0/128.0; // we send data as int, but display it as float

  // Measure plant 1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  // Measure plant 1 %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // measure resistance and convert to better scale (see note below)
  uint32_t currentRes2 = measure_resistance(1);
  fullRes2 = (currentRes2 + fullRes2)/2; // moving average

  // convert full Res to log and multiply, this way we use 16 bit resolution better
  // and linearize the scale a bit...
  NanoSensorData.soilRes2 = 10000*log10(fullRes2);
    // NOTE resistance is in range between 1 and 10^6, so log(10) of resistance
    // is betweem 1 and 6, but we send uint16 integers, with a range of 0-65,535
    // so we take log10 and multiply by 10, thus having a nice, and 'linear'
    // scale with a descent resolution

  // measure one-wire temperature
  Sensors.requestTemperatures();
  Sensors.getAddress(tempDeviceAddress, 1);
  NanoSensorData.temp2 = Sensors.getTemp(tempDeviceAddress);
  float tempCel2 = NanoSensorData.temp2*1.0/128.0; // we send data as int, but display it as float

  Serial.print("Humidity:    ");
  Serial.print(moistSense.readHumidity(), 2);
  Serial.print("\tTemperature: ");
  Serial.println(moistSense.readTemperature(), 2);

  // for (uint8_t pumpPower = 0; pumpPower < 11; pumpPower++){
  //   analogWrite(PUMP_PIN_1, pumpPower*255/10);          // set the voltage supply on
  //   Serial.print("pumping with power: ");
  //   Serial.println(pumpPower*255/10);
  //   delay(500);
  // }
  analogWrite(PUMP_PIN_2, 50);          // set the voltage supply on
  delay(10000);
  analogWrite(PUMP_PIN_2, 255);          // set the voltage supply on
  delay(10000);

  // if(answeredMasterRequest || true)
  // {
    Serial.println("----------------------------------------------------------");
    Serial.println("[SLAVE] Send data to master!");
    Serial.print("[SLAVE] Measured resistance 1: ");
    Serial.println(currentRes1);
    Serial.print("[SLAVE] Averaged resistance 1: ");
    Serial.println(fullRes1);
    Serial.print("[SLAVE] Calculated linear resistance 1: ");
    Serial.println(NanoSensorData.soilRes1);
    Serial.print("[SLAVE] Measured temperature 1: ");
    Serial.println(tempCel1);
    Serial.println();
    Serial.print("[SLAVE] Measured resistance 1: ");
    Serial.println(currentRes2);
    Serial.print("[SLAVE] Averaged resistance 1: ");
    Serial.println(fullRes2);
    Serial.print("[SLAVE] Calculated linear resistance 1: ");
    Serial.println(NanoSensorData.soilRes2);
    Serial.print("[SLAVE] Measured temperature 1: ");
    Serial.println(tempCel2);
    Serial.println();
    answeredMasterRequest = false;
  // }

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

void setupCurrentPath(uint8_t iPath, uint8_t changeCurrentPath) {
  // setup current paths for nPaths, where each path is connected to their own
  // moisture sensing gypsum sensor
  switch (iPath)
  {
    case 0: // first current path
      if (doReadForward)
      {
        activeDigitalPin = MOIST_SUPPL_2;
        supplyVoltageAnalogPin = MOIST_SENSE_2;
        sensorVoltageAnalogPin = MOIST_SENSE_1;
      }
      else {
        activeDigitalPin = MOIST_SUPPL_1;
        supplyVoltageAnalogPin = MOIST_SENSE_1;
        sensorVoltageAnalogPin = MOIST_SENSE_2;
      }
      break;
    case 1:
      if (doReadForward)
      {
        activeDigitalPin = MOIST_SUPPL_3;
        supplyVoltageAnalogPin = MOIST_SENSE_3;
        sensorVoltageAnalogPin = MOIST_SENSE_4;
      }
      else {
        activeDigitalPin = MOIST_SUPPL_4;
        supplyVoltageAnalogPin = MOIST_SENSE_4;
        sensorVoltageAnalogPin = MOIST_SENSE_3;
      }
      break;
    default:
      break;
  }

  if (changeCurrentPath)
    doReadForward != doReadForward;
}


int32_t measure_resistance(uint8_t currentPath){
  // Serial.print("[SLAVE] Measuring soil resistance...");
  // http://vanderleevineyard.com/vineyard-blog/-the-vinduino-project-3-make-a-low-cost-soil-moisture-sensor-reader
  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter

  int32_t resistance = 0; // reset resistance
  int32_t tempResistance = 0; // reset resistance
  for (int i=0; i < N_READS; i++)
  {
    setupCurrentPath(currentPath,1);      // Prepare the digital and analog pin values

    digitalWrite(activeDigitalPin, HIGH);                 // set the voltage supply on
    delay(10);
    int16_t supplyReadOut = analogRead(supplyVoltageAnalogPin);   // read the supply voltage
    int16_t sensorReadOut = analogRead(sensorVoltageAnalogPin);   // read the sensor voltage
    digitalWrite(activeDigitalPin, LOW);                  // set the voltage supply off
    delay(10);

    int16_t diffRead = supplyReadOut - sensorReadOut;
      // make sure we don't have short, in which case both values are very close
    if ((sensorReadOut >= 3) & (diffRead>3))
      tempResistance = int32_t(float(COMP_RESISTOR)*(supplyReadOut-sensorReadOut)/sensorReadOut);
    else if ((sensorReadOut >= 3) & !(diffRead>3)) // short, so set res close to zero
      tempResistance = 1; // set to 1M ohm as upper limit
    else
      tempResistance = MEGA*3; // set to 1M ohm as upper limit
        // if sensor readout becomes too small, resistance values become meaningless
        // so set upper limit for possible resistance
    resistance += tempResistance;
  }

  resistance = resistance/N_READS;
  if (resistance > MEGA*3)
    resistance = MEGA*3; // set to mega ohm

  return resistance;
}
>>>>>>> master
