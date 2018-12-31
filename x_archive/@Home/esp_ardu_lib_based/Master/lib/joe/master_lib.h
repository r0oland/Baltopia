#ifndef MASTER_LIB_H
#define MASTER_LIB_H

// Always need Arduino & some communication stuff
#include <Arduino.h>
#include <Wire.h> // I2C

// Standard C libraries
#include <string.h>
#include <stdio.h>
#include <time.h>

// Special or Custom libraries
#include <../EasyTransferI2C/EasyTransferI2C.h>


// communication constants
const uint32_t SERIAL_SPEED = 9600; // needs to be prgrammed into the ESP, default is higher

// I2C communication
const uint8_t PROTOCOL_OVERHEAD = 4; // 4 bytes, 0x06, 0x85, size and checksum
//define slave i2c address
const uint8_t I2C_SLAVE_ADDRESS = 9;
const uint8_t MAX_I2C_STRUCT_SIZE = 26;

struct DataStruct{
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int8_t i8 = 0;
  uint8_t ui8 = 0;
  int16_t i16 = 0;
};

const uint8_t I2C_SDA_PIN = D2;
const uint8_t I2C_SCL_PIN = D1;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// function declarations
void checkStructSize(HardwareSerial &serialRef, uint8_t structSize);

#endif
