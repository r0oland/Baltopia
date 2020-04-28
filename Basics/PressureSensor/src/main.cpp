#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <AdafruitIO_WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <FastLED.h>

#include "secrets.h"


//void print_values_serial();
void print_values_serial(Adafruit_BME280 *bmeSensor);
void send_aio_values(Adafruit_BME280 *insideSensor, Adafruit_BME280 *outsideSensor);
void updateMinHumid(AdafruitIO_Data *data);
void updateMaxHumid(AdafruitIO_Data *data);
// void lcd_update_humid_info();

Adafruit_BME280 RightBME; // I2C
Adafruit_BME280 LeftBME;  // I2C

uint16_t delayTime = 1000;
const uint8_t INSIDE_ADDRESS = 0x77;
const uint8_t OUTSIDE_ADDRESS = 0x76;

// setup adafruit stuff
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PWD);
AdafruitIO_Feed *bigTeraHumid = io.feed("a_big_tera_humid");
AdafruitIO_Feed *smallTeraHumid = io.feed("a_small_tera_humid");
AdafruitIO_Feed *bigTeraTemp = io.feed("a_big_tera_temp");
AdafruitIO_Feed *smallTerraTemp = io.feed("a_small_tera_temp");
AdafruitIO_Feed *minHumidFeed = io.feed("a_min_humid");
AdafruitIO_Feed *maxHumidFeed = io.feed("a_max_humid");

uint8_t humidRunning = false;
// uint32_t humdOnTime = 0;             // will store last time LED was updated
// const uint32_t humidOnTime = 30000;  // keep moisture on for this long
float minRequiredOnHumid = 0;  // keep at least this much humidity
float minRequiredOffHumid = 0; // humid until we reach this value

void setup()
{
  Serial.begin(921600);
  while (!Serial)
    ; // wait for serial monitor to open
  Serial.println("");
  Serial.println("");

  // lcd.init(); // initialize the lcd
  // lcd.backlight();

  // setting up the sensors ====================================================
  // lcd.setCursor(0, 0);
  // lcd.print("Setting up sensors:");
  // lcd.setCursor(0, 1);
  // lcd.print("Right sensor...");
  Serial.print("Looking for right humid. sensor...");
  if (!RightBME.begin(INSIDE_ADDRESS, &Wire))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
  Serial.println("found!");
  // lcd.print("OK!");

  // lcd.setCursor(0, 2);
  // lcd.print(" Left sensor...");
  Serial.print("Looking for left humid. sensor...");
  if (!LeftBME.begin(OUTSIDE_ADDRESS, &Wire))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
  Serial.println("found!");
  // lcd.print("OK!");
  delay(2000);

  // connect to io.adafruit.com
  // lcd.clear();
  // lcd.setCursor(0, 0);
  Serial.print("Connecting to Adafruit IO");
  // lcd.print("Connecting to AIO");
  io.connect();
  minHumidFeed->onMessage(updateMinHumid);
  maxHumidFeed->onMessage(updateMaxHumid);

  // wait for a connection
  // lcd.setCursor(0, 1);
  while (io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    // lcd.print(".");
    delay(300);
  }
  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  send_aio_values(&RightBME, &LeftBME);
  minHumidFeed->get();
  maxHumidFeed->get();

  // setup my own IO pins
  pinMode(D0, OUTPUT); // relais for fan and humidifier
  digitalWrite(D0, 1);
  pinMode(D5, INPUT); // button pin

  // lcd.clear();
  // lcd.setCursor(0, 0);
  // lcd.print("R: ");
  // lcd.setCursor(0, 1);
  // lcd.print("L: ");
  // lcd.setCursor(0, 3);
  // lcd.print("IO status: ");

  // lcd.print("HuOff L:100% H:100%");
  // lcd_update_humid_info();
}

void loop()
{
  io.run(); // io.run(); is required for all sketches.

  EVERY_N_SECONDS(5) // print lcd and serial values...
  {
    Serial.println("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
    Serial.println("Right Terarium:");
    RightBME.takeForcedMeasurement();
    print_values_serial(&RightBME);
    // lcd.setCursor(3, 0);
    // // print_values_LCD(&RightBME, &lcd);

    Serial.println("Left Terarium:");
    LeftBME.takeForcedMeasurement();
    print_values_serial(&LeftBME);
    // lcd.setCursor(3, 1);
    // // print_values_LCD(&LeftBME, &lcd);

    Serial.print("Humid running: ");
    Serial.println(humidRunning);
  }

  EVERY_N_SECONDS(20) // send AIO values and update AIO status
  {
    send_aio_values(&RightBME, &LeftBME);
    // lcd.setCursor(11, 2);
    // lcd.print(io.status());
  }

  EVERY_N_SECONDS(10)
  {
    // get latest humidity reading
    RightBME.takeForcedMeasurement();
    LeftBME.takeForcedMeasurement();
    float rightHumid = RightBME.readHumidity();
    float leftHumid = LeftBME.readHumidity();

    // if one is to low, turn on humidifier
    bool leftHumidToLow = (leftHumid < minRequiredOnHumid);
    bool rightHumidToLow = (rightHumid < minRequiredOnHumid);
    bool humidToLow = rightHumidToLow || leftHumidToLow;

    // if both are high enough, turn off humidifier
    bool leftHumidReached = (leftHumid > minRequiredOffHumid);
    bool rightHumidReached = (rightHumid > minRequiredOffHumid);
    bool humidReached = leftHumidReached && rightHumidReached;
    if (humidToLow && !humidRunning)
    {
      // humdOnTime = millis();
      humidRunning = true;
      digitalWrite(D0, 0);
      Serial.println("Turned ON humidifier");
    }
    else if (humidRunning && humidReached)
    {
      humidRunning = false;
      digitalWrite(D0, 1);
      Serial.println("Turned OFF humidifier");
    }
    // lcd_update_humid_info();
  }
}

void print_values_serial(Adafruit_BME280 *bmeSensor)
{
  Serial.println("-------------------------------------------");
  Serial.print("Temperature = ");
  Serial.print(bmeSensor->readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bmeSensor->readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bmeSensor->readHumidity());
  Serial.println(" %");

  Serial.println("-------------------------------------------");
}

// // void print_values_LCD(Adafruit_BME280 *bmeSensor, LiquidCrystal_I2C *lcd)
// {
// lcd->print("");
// lcd->print(bmeSensor->readTemperature());
// lcd->print("*C ");

// lcd->print(bmeSensor->readHumidity());
// lcd->print("%");
// }

void send_aio_values(Adafruit_BME280 *insideSensor, Adafruit_BME280 *outsideSensor)
{
  bigTeraHumid->save(insideSensor->readHumidity());
  bigTeraTemp->save(insideSensor->readTemperature());

  smallTeraHumid->save(outsideSensor->readHumidity());
  smallTerraTemp->save(outsideSensor->readTemperature());
}

void updateMinHumid(AdafruitIO_Data *data)
{
  // convert the data to integer
  minRequiredOnHumid = data->toFloat();

  Serial.print("New minimum Humidity: ");
  Serial.println(minRequiredOnHumid);
  // lcd_update_humid_info();
}

void updateMaxHumid(AdafruitIO_Data *data)
{
  // convert the data to integer
  minRequiredOffHumid = data->toFloat();

  Serial.print("New maximum Humidity: ");
  Serial.println(minRequiredOffHumid);
  // lcd_update_humid_info();
}

// void lcd_update_humid_info()
// {
//   // lcd.setCursor(0, 2);
//   // lcd.print("                    ");
//   // lcd.setCursor(0, 2);
//   if (humidRunning)
//   {
//     // lcd.print("HuOn ");
//   }
//   else
//   {
//     // lcd.print("HuOff");
//   }
//   // lcd.setCursor(6, 2);
//   // lcd.print("L:");
//   // lcd.print((uint16_t)minRequiredOnHumid);
//   // lcd.print("%");
//   // lcd.setCursor(13, 2);
//   // lcd.print("H:");
//   // lcd.print((uint16_t)minRequiredOffHumid);
//   // lcd.print("%");
// }