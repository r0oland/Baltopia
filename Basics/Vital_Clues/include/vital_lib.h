#ifndef VITAL_LIB
#define VITAL_LIB

#include <Arduino.h> // always required when using platformio
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "..\include\secrets.h"

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// Pin definitions
const uint8_t ONE_WIRE_BUS = D3; // temp. sensor connected on D2
const uint8_t MOUSE_PAD_PIN = D4; // transistor controll pin
const uint8_t MOUSE_ROOM_PIN = D5; // transistor controll pin
const uint8_t THERMISTORPIN = A0;

// % LCD related settings and stuff...------------------------------------------
const uint8_t LCD_ADDRESS = 0x27;
const uint8_t LCD_COLS = 40;
const uint8_t LCD_ROWS = 2;

uint8_t lcd_clock[8] = {0x0, 0xe, 0x15, 0x17, 0x11, 0xe, 0x0};
uint8_t heart[8] = {0x0, 0xa, 0x1f, 0x1f, 0xe, 0x4, 0x0};
uint8_t check[8] = {0x0, 0x1 ,0x3, 0x16, 0x1c, 0x8, 0x0};

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

// General Function Declarations
void setup_serial();

// Vital Class Definition %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
class Vital
{
  public:
    Vital(uint8_t oneWireBus, uint8_t lcd_addr, uint8_t lcd_cols, uint8_t lcd_rows):
      V_OneWire(oneWireBus),
      V_LCD(lcd_addr,lcd_cols,lcd_rows),
      V_TempSensor(&V_OneWire)
      {
        for (iErr = 0; iErr < nInt; iErr++)  // reset error container to 0s
          errContainer[iErr] = 0;
        iErr = 0;
      };

    void setup_lcd();
    void setup_io_pins();
    void setup_temp_sensor(uint8_t sensReso);
    void get_temp_sensor_address();
    void update_serial();
    void get_analog_temp();
    void get_digital_temp();
    void control_heat_pads();
    void update_lcd(bool iotConnected);

    LiquidCrystal_I2C V_LCD;
    OneWire V_OneWire;
    DallasTemperature V_TempSensor;
    DeviceAddress* PadTempSens;
    DeviceAddress* RoomTempSens;
    DeviceAddress* AmbientTempSens;

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // variables for analog & digital temperature measurements
    const uint16_t THERMISTORNOMINAL = 10000;
    const float TEMPERATURENOMINAL = 25.0;
    const uint8_t N_SAMPLES = 10;
    const uint16_t BCOEFFICIENT = 3960;
    const uint16_t SERIESRESISTOR = 9970;
    const float faultTemperature = -120; // TODO make sure to use this!

    float targetTemperature = 37.0;
    float maxPadTemp = 45.0;
    float analTemp = 0.0; // temperature measured in anus
    float padTemp = 0.0;
    float roomTemp = 0.0;
    float ambTemp = 0.0;

    //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // PID variables
    float err; // difference between measured and set point
    float previous_err = 0;
    float integral = 0;
    float derivative = 0;
    const uint_fast8_t nInt = 20;
    float errContainer[20];
    unsigned char iErr; // runs from 1 to 10
    const float pValue = 90.0; // proportional part of controller
    const float dValue = 0; // differential part of controller
    const float iValue = 10; // integral part of controller
    float pwmValue = 0; // set initial heating to 0
    int iPwmValue = 0; // inted pwm value

  private:
};


#endif
