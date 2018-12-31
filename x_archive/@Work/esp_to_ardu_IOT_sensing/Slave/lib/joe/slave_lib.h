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
// A0 - moisture sensor
// A4 & A5 used for I2C

// communication constants
const uint32_t SERIAL_SPEED = 9600; // needs to be prgrammed into the ESP, default is higher

const uint8_t I2C_SLAVE_ADDRESS = 9;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// function declarations

// I2C interrupt functions
void requestEvent();
void receiveEvent(int howMany);

#endif
