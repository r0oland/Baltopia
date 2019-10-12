#include <Baltopia.h> // send data stucts, see in shared libs folder
#include "..\lib\Joe\slave_lib.h"

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
  Serial.begin(SERIAL_SPEED);
  Serial.println("===========================================================");
  Serial.println("[SLAVE] Ready to go!");

  realRes = measure_resistance()/100;
    // measure once to get startign value for movign avarage in loop()
    // divide by 100 in order to go from 1 M ohm to 10k Ohm which can be stored as uint16
}

////////////////////////////////////////////////////////////////////////////////
void loop() {
  realRes = (measure_resistance()/100 + realRes)/2; // moving average
  NanoSensorData.soilRes = realRes;
  NanoSensorData.status += 1;
  delay(500);

  if(answeredMasterRequest)
  {
    Serial.println("[SLAVE] Send data to master!");
    Serial.print("[SLAVE] Measured resistance: ");
    Serial.println(NanoSensorData.soilRes);
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

  Serial.print("average: ");
  Serial.println(resistance);

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
