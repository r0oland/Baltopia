#ifndef SLAVE_LIB_H
#define SLAVE_LIB_H

#include <Arduino.h>
#include <Baltopia.h> // send data stucts, see in shared libs folder

// define global variables %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// use a few as possible
uint8_t answeredMasterRequest = false;
uint8_t supplyVoltageAnalogPin;
uint8_t sensorVoltageAnalogPin;

uint32_t fullRes1;
uint32_t fullRes2;
uint8_t activeDigitalPin;
uint8_t doReadForward = 0; // switch between current during analog read of moisture sensor

EasyTransferI2C ET;

//give a name to the group of data
SensorData NanoSensorData;

// define general constants %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const uint8_t N_READS = 10;    // Number of sensor reads for filtering
const uint16_t COMP_RESISTOR = 21100;  // Constant value of known resistor in Ohms
const uint32_t MEGA = 1000000;  // Constant value of known resistor in Ohms

// Sensor libraries
// #include <Adafruit_Sensor.h>
// #include <DHT.h>
// #include "OneWire.h"
// #include "DallasTemperature.h"


// DEFINE ANALOG PINS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// A0 - moisture sensor
// A4 & A5 used for I2C
const uint8_t MOIST_SENSE_1 = A3;
const uint8_t MOIST_SENSE_2 = A2;
const uint8_t MOIST_SENSE_3 = A1;
const uint8_t MOIST_SENSE_4 = A0;


// DEFINE DIGITAL PINS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// D0 & D1 - used for serial?
const uint8_t MOIST_SUPPL_1 = 2; // -127 = no pull down, not connected
const uint8_t MOIST_SUPPL_2 = 3; // -127 = no pull down, not connected
const uint8_t MOIST_SUPPL_3 = 4; // -127 = no pull down, not connected
const uint8_t MOIST_SUPPL_4 = 5; // -127 = no pull down, not connected

// PWM pins for pump control,  possible are 3, 5, 6, 9, 10
const uint8_t PUMP_PIN_1 = 6; // -127 = no pull down, not connected
const uint8_t PUMP_PIN_2 = 9; // -127 = no pull down, not connected

// const uint8_t DHT_PIN = 2; // digital pin DHT is connected to
// const uint8_t ONE_WIRE_BUS = 3;
// const uint8_t TEMP_RELAIS_PIN = 4;
// const uint8_t LIGHT_RELAIS_PIN = 5;
// const uint8_t PUMP_RELAIS_PIN = 5;
// const uint8_t LIGHT_BUTTON_PIN = 6;
//
// // sensor consts
// const uint8_t DHT_TYPE = 22; // DHT22, see DTH.h

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// function declarations
void receive(int numBytes) {};
void data_request_from_master();
void setupCurrentPath(uint8_t iPath, uint8_t changeCurrentPath);
int32_t measure_resistance(uint8_t currentPath);


#endif
