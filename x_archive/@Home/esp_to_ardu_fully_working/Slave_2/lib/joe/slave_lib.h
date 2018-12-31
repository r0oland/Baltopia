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

// DEFINE DIGITAL PINS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// D0 & D1 - used for serial?
const uint8_t DHT_PIN = 2; // digital pin DHT is connected to
const uint8_t ONE_WIRE_BUS = 3;
const uint8_t TEMP_RELAIS_PIN = 4;
const uint8_t LIGHT_RELAIS_PIN = 5;
const uint8_t PUMP_RELAIS_PIN = 5;
const uint8_t LIGHT_BUTTON_PIN = 6;

// sensor consts
const uint8_t DHT_TYPE = 22; // DHT22, see DTH.h

// communication constants
const uint32_t SERIAL_SPEED = 9600; // needs to be prgrammed into the ESP, default is higher
const byte SLAVE_ADDRESS = 42;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// function declarations

// send 8,16 and 32 bit numbers over I2C to master
// send char over I2C to master

// recieve char over I2C from maser

// TODO - implement these functions as well?
// slave ready command

#endif
