#ifndef BALTOPIA_h
#define BALTOPIA_h

#include <Arduino.h>

// Special or Custom libraries
#include <Wire.h>
// https://github.com/madsci1016/Arduino-EasyTransfer

#include <EasyTransferI2C.h> // send data stucts, see in shared libs folder

// Standard C libraries
// #include <string.h>
// #include <stdio.h>
// #include <time.h>

const uint32_t SERIAL_SPEED = 9600; // needs to be prgrammed into the ESP, default is higher
const float INT_TO_TEMPERATURE = 1.0/128.0; // needs to be prgrammed into the ESP, default is higher

struct SensorData{
  // int8_t status = 0; // to be used for setting things (pump, light, etc...)
  uint16_t status = 0;
  uint16_t soilRes1 = 0;
  uint16_t soilRes2 = 0;
  // one wire temperature sensor returns temperature raw value
  // (12 bit integer of 1/128 degrees C)
  int16_t temp1 = 0;
  int16_t temp2 = 0;
};

const uint8_t PROTOCOL_OVERHEAD = 4; // 4 bytes, 0x06, 0x85, size and checksum
const uint8_t MAX_I2C_STRUCT_SIZE = 26;
const uint8_t I2C_SLAVE_ADDRESS = 10;
const uint8_t I2C_SLAVE2_ADDRESS = 11;

#endif
