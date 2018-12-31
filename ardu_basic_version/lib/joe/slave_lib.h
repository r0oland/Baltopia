#ifndef SLAVE_LIB_H
#define SLAVE_LIB_H

// Always need Arduino & some communication stuff
#include <Arduino.h>
#include <Wire.h> // I2C

// Standard C libraries
#include <string.h>
#include <stdio.h>
#include <time.h>

// Sensor libraries
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include "OneWire.h"
#include "DallasTemperature.h"

// Special or Custom libraries
#include <Wire.h>
// https://github.com/madsci1016/Arduino-EasyTransfer
#include <../EasyTransferI2C/EasyTransferI2C.h>


// DEFINE ANALOG PINS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

// communication related %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
const uint32_t SERIAL_SPEED = 9600; // needs to be prgrammed into the ESP, default is higher
const uint8_t I2C_SLAVE_ADDRESS = 9;

// communication constants
struct DataStruct{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int8_t i8 = 0;
  uint8_t ui8 = 0;
  int16_t i16 = 0;
};


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// function declarations

void receive(int numBytes) {};
void espWifiRequestEvent();

#endif
