#ifndef MASTER_LIB_H
#define MASTER_LIB_H

#include <Arduino.h>
#include <Baltopia.h> // send data stucts, see in shared libs folder
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include "secrets.h"

//create object
EasyTransferI2C ET;

//give a name to the group of data
SensorData NanoSensorData;


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// function declarations
void receive(size_t numBytes) {};

#endif
