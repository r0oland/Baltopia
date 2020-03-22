#include "vital_lib.h"

// define general functions here
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// setup serial
void setup_serial(){
  Serial.begin(9600);
  Serial.println();
  Serial.println("-------------------------------------------------------------");
  Serial.println("-----------------------   Vital Clues   ---------------------");
  Serial.println("--------------------   Ver.01 | JR | 2019  ------------------");
  Serial.println("-------------------------------------------------------------");
  delay(1000);
}

// define Vital methods here
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// setup IO PINs
void Vital::setup_io_pins(){
  pinMode(MOUSE_PAD_PIN, OUTPUT); // set relay pin as digital output
  pinMode(MOUSE_ROOM_PIN, OUTPUT); // set relay pin as digital output
  analogWrite(MOUSE_PAD_PIN, 0);
  analogWrite(MOUSE_ROOM_PIN, 0);
}

// function to read out smaller sensor placed in anus of animal
void Vital::get_analog_temp(){
  float average = 0;
  // take N samples in a row and sum them up
  for (int i = 0; i< N_SAMPLES; i++)
    average += analogRead(THERMISTORPIN);
  average /= N_SAMPLES;

  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;

  analTemp = average / THERMISTORNOMINAL;     // (R/Ro)
  analTemp = log(analTemp);                  // ln(R/Ro)
  analTemp /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  analTemp += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  analTemp = 1.0 / analTemp;                 // Invert
  analTemp -= 273.15;                         // convert to C
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// function to read out digital sensor placed on pad
void Vital::get_digital_temp(){
  // update temperature readings.
  V_TempSensor.requestTemperatures();
  padTemp = V_TempSensor.getTempC(*PadTempSens);
  roomTemp = V_TempSensor.getTempC(*RoomTempSens);
  ambTemp = V_TempSensor.getTempC(*AmbientTempSens);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// function to read out digital sensor placed on pad
void Vital::control_heat_pads(){
  this->get_analog_temp();
  this->get_digital_temp();

  // control mouse heat pad using PID ------------------------------------------
  err = targetTemperature - analTemp; // calculate error
  errContainer[iErr] = err;
  if (iErr >= nInt)
    iErr = 0;
  // calculate integral over last 10 measurements
  integral = 0;
  for(unsigned char i = 0; i < nInt; i++)
    integral += errContainer[i];
  // integral = integral + err; // update integral value

  derivative = err - previous_err; // update derivative error
  pwmValue = pValue * err + dValue * derivative + iValue * integral;
  previous_err = err;

  iPwmValue = pwmValue;
  if (pwmValue < 0){
    pwmValue = 0;
  }

  if (pwmValue > 255){
    pwmValue = 255;
  }

  // if pad termperature is too high, disable, otherwise we carbonize the mouse
  if (padTemp > maxPadTemp)
    pwmValue = 0;

  analogWrite(MOUSE_PAD_PIN,pwmValue);

  // control mouse room, rought control is fine...
  if (roomTemp > 35.0)
    analogWrite(MOUSE_ROOM_PIN,0);
  else
    analogWrite(MOUSE_ROOM_PIN,255);

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// setup small LCD
void Vital::setup_lcd(){
  V_LCD.init();                      // initialize the lcd
  V_LCD.backlight();
  V_LCD.createChar(0, lcd_clock);
  V_LCD.createChar(1, heart);
  V_LCD.createChar(2, check);
  V_LCD.clear();
  V_LCD.home();
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// do stuff here!
void Vital::update_lcd(bool iotConnected){
  // FIXME use sprintf instead!
  // https://arduinobasics.blogspot.com/2019/05/sprintf-function.html
  V_LCD.setCursor(0, 0);
  V_LCD.print("AIO:");
  if (iotConnected){
    V_LCD.printByte(2);
  }
  else {
    V_LCD.print("X");
  }

  V_LCD.setCursor(6, 0);
  char data[40];
  sprintf(data, " M:%02.1f P:%02.1f R:%02.1f Pw:%03.0f%%       ",
    analTemp,padTemp,roomTemp,pwmValue/255*100);
  V_LCD.print(data);
  V_LCD.setCursor(0, 1);
  sprintf(data, "Ambient:%02.1f          ",ambTemp);
  V_LCD.print(data);
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// do stuff here!
void Vital::update_serial(){
  // FIXME use sprintf instead!
  // https://arduinobasics.blogspot.com/2019/05/sprintf-function.html
  // serial print values
  Serial.print(analTemp);
  Serial.print(" ");
  Serial.print(padTemp);
  Serial.print(" ");
  Serial.print(pwmValue);
  Serial.print(" ");
  Serial.println(targetTemperature);
}


// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void Vital::setup_temp_sensor(uint8_t sensReso){
  V_TempSensor.begin();   // Start MyVital.V_TempSensor
  V_TempSensor.setResolution(sensReso); // reduce resolution to reduce noise
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void Vital::get_temp_sensor_address(){
  DeviceAddress tempSensor;

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(V_TempSensor.getDeviceCount(), DEC);
  Serial.println(" devices.");
  if (!V_TempSensor.getAddress(tempSensor, 0))
    Serial.println("Unable to find address for Device 0");

  // show the addresses we found on the bus
  Serial.print("Device Address: ");
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (tempSensor[i] < 16) Serial.print("0");
    Serial.print(tempSensor[i], HEX);
  }
  Serial.println();
}
