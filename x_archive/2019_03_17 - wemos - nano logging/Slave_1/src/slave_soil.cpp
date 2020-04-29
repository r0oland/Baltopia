#include <Baltopia.h> // send data stucts, see in shared libs folder
#include "..\lib\Joe\slave_lib.h"

#include "OneWire.h" // OneWire by Paul Stoffregen
#include "DallasTemperature.h" // DallasTemperature by Guil Barros
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address
void printAddress(DeviceAddress deviceAddress);
const uint8_t ONE_WIRE_BUS = 12; // -127 = no pull down, not connected
OneWire OneWire(ONE_WIRE_BUS);
DallasTemperature Sensors(&OneWire);

////////////////////////////////////////////////////////////////////////////////
void setup(){
  // setup digital output pins
  pinMode(MOIST_SUPPL_1, OUTPUT);
  pinMode(MOIST_SUPPL_2, OUTPUT);

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


  fullRes = measure_resistance();
    // measure once to get startign value for movign avarage in loop()
}

////////////////////////////////////////////////////////////////////////////////
void loop() {

  // measure resolution and convert to better scale (see note below)
  fullRes = (measure_resistance() + fullRes)/2; // moving average
  // convert full Res to log and multiply, this way we use 16 bit resolution better
  // and linearize the scale a bit...
  uint16_t linearResistance = 1000*log10(fullRes);
    // NOTE resistance is in range between 1 and 10^6, so log(10) of resistance
    // is betweem 1 and 6, but we send uint16 integers, with a range of 0-65,535
    // so we take log10 and multiply by 10, thus having a nice, and 'linear'
    // scale with a descent resolution

  // measure one-wire temperature
  Sensors.requestTemperatures();
  Sensors.getAddress(tempDeviceAddress, 0);
  int16_t rawTemperature = Sensors.getTemp(tempDeviceAddress);
  // save all measurements to the sensor data struct which is called by the
  // wemos master in regular intervals
  NanoSensorData.status += 1;
  NanoSensorData.temp1 = rawTemperature;
  NanoSensorData.soilRes1 = linearResistance;
  delay(2000);

  if(answeredMasterRequest)
  {
    Serial.println("[SLAVE] Send data to master!");
    Serial.print("[SLAVE] Measured resistance: ");
    Serial.println(NanoSensorData.soilRes1);
    Serial.print("[SLAVE] Measured temperature: ");
    Serial.println(rawTemperature*1.0/128.0);
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


int32_t measure_resistance(){
  // Serial.print("[SLAVE] Measuring soil resistance...");
  // http://vanderleevineyard.com/vineyard-blog/-the-vinduino-project-3-make-a-low-cost-soil-moisture-sensor-reader
  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter

  int32_t resistance = 0; // reset resistance
  int32_t tempResistance = 0; // reset resistance
  for (int i=0; i < N_READS; i++) {
    setupCurrentPath();      // Prepare the digital and analog pin values

    digitalWrite(activeDigitalPin, HIGH);                 // set the voltage supply on
    delay(10);
    int16_t supplyReadOut = analogRead(supplyVoltageAnalogPin);   // read the supply voltage
    int16_t sensorReadOut = analogRead(sensorVoltageAnalogPin);   // read the sensor voltage
    digitalWrite(activeDigitalPin, LOW);                  // set the voltage supply off
    delay(10);

    if (sensorReadOut >= 2)
      tempResistance = int32_t(float(COMP_RESISTOR)*(supplyReadOut-sensorReadOut)/sensorReadOut);
    else
      tempResistance = MEGA; // set to 1M ohm as upper limit
        // if sensor readout becomes too small, resistance values become meaningless
        // so set upper limit for possible resistance

    // Serial.print(supplyReadOut);
    // Serial.print(" ");
    // Serial.print(sensorReadOut);
    // Serial.print(" ");
    //
    // Serial.print(tempResistance);
    // Serial.println(" ");
    resistance += tempResistance;
  }

  resistance = resistance/N_READS;
  if (resistance > MEGA)
    resistance = MEGA; // set to mega ohm

  return resistance;
}

void setupCurrentPath() {
  if ( activeDigitalPin == MOIST_SUPPL_1 ) {
    activeDigitalPin = MOIST_SUPPL_2;
    supplyVoltageAnalogPin = MOIST_SENSE_2;
    sensorVoltageAnalogPin = MOIST_SENSE_1;
  }
  else {
    activeDigitalPin = MOIST_SUPPL_1;
    supplyVoltageAnalogPin = MOIST_SENSE_1;
    sensorVoltageAnalogPin = MOIST_SENSE_2;
  }
}
