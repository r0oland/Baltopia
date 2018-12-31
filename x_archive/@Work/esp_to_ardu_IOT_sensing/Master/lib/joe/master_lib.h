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
#include <Wire.h>
#include <../EasyTransferI2C/EasyTransferI2C.h>

// communication constants
const uint32_t SERIAL_SPEED = 9600; // needs to be prgrammed into the ESP, default is higher

const uint8_t PROTOCOL_OVERHEAD = 4; // 4 bytes, 0x06, 0x85, size and checksum
const uint8_t I2C_SLAVE_ADDRESS = 9;
const uint8_t MAX_I2C_STRUCT_SIZE = 26;

// connect to ze interwebs
const char* SSID = "HMGU-Guests";                        // Wireless SID
const char* WIFI_PASS = "wifi4guests";                // Wireless Passcode

unsigned int myChannelNumber = 401628;
const char * myWriteAPIKey = "SSQ9JLXIY00AUK54";

// I2C interrupt functions
// void requestEvent();
// void receiveEvent(int howMany);

#endif
