#include <Arduino.h>
// #include <WiFi.h> % for ESP32
#include <ESP8266WiFi.h>
#include <AdafruitIO_WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal_I2C.h>
#include <FastLED.h>

#include "secrets.h"

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

//void print_values_serial();
void print_values_serial(Adafruit_BME280 *bmeSensor);
void print_values_LCD(Adafruit_BME280 *bmeSensor, LiquidCrystal_I2C *lcd);
void send_aio_values(Adafruit_BME280 *insideSensor, Adafruit_BME280 *outsideSensor);

Adafruit_BME280 RightBME; // I2C
Adafruit_BME280 LeftBME;  // I2C

uint16_t delayTime = 1000;
const uint8_t INSIDE_ADDRESS = 0x77;
const uint8_t OUTSIDE_ADDRESS = 0x76;

// setup adafruit stuff
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PWD);
AdafruitIO_Feed *teraInHum = io.feed("teraInHum");
AdafruitIO_Feed *teraOuHum = io.feed("teraOuHum");
AdafruitIO_Feed *teraInTemp = io.feed("teraInTemp");
AdafruitIO_Feed *teraOutTemp = io.feed("teraOutTemp");

uint8_t humidRunning = false;
// uint32_t humdOnTime = 0;             // will store last time LED was updated
// const uint32_t humidOnTime = 30000;  // keep moisture on for this long
const float minRequiredOnHumid = 75; // keep at least 75% humidity
const float minRequiredOffHumid = 90;
// if humid is running, keep it running until we reach this value

void setup()
{
  Serial.begin(921600);
  while (!Serial)
    ; // wait for serial monitor to open
  Serial.println("");
  Serial.println("");

  lcd.init(); // initialize the lcd
  lcd.backlight();

  // setting up the sensors ====================================================
  lcd.setCursor(0, 0);
  lcd.print("Setting up sensors:");
  lcd.setCursor(0, 1);
  lcd.print("Right sensor...");
  Serial.print("Looking for right humid. sensor...");
  if (!RightBME.begin(INSIDE_ADDRESS, &Wire))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
  Serial.println("found!");
  lcd.print("found!");

  lcd.setCursor(0, 2);
  lcd.print("Left sensor...");
  Serial.print("Looking for left humid. sensor...");
  if (!LeftBME.begin(OUTSIDE_ADDRESS, &Wire))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
  Serial.println("found!");
  lcd.print("found!");
  delay(2000);

  // connect to io.adafruit.com
  lcd.clear();
  lcd.setCursor(0, 0);
  Serial.print("Connecting to Adafruit IO");
  lcd.print("Connecting to AIO");
  io.connect();

  // wait for a connection
  lcd.setCursor(0, 1);
  while (io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    lcd.print(".");
    delay(300);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  send_aio_values(&RightBME, &LeftBME);

  // setup my own IO pins
  pinMode(D0, OUTPUT); // relais for fan and humidifier
  digitalWrite(D0, 1);
  pinMode(D5, INPUT); // button pin

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" In: ");
  lcd.setCursor(0, 1);
  lcd.print("Out: ");
  lcd.setCursor(0, 2);
  lcd.print("IO status: ");
  lcd.setCursor(0, 3);
  lcd.print("Humid: ");
}

void loop()
{
  io.run(); // io.run(); is required for all sketches.

  EVERY_N_SECONDS(5) // print lcd and serial values...
  {
    Serial.println("Right Terarium:");
    RightBME.takeForcedMeasurement();
    print_values_serial(&RightBME);
    lcd.setCursor(5, 0);
    print_values_LCD(&RightBME, &lcd);

    Serial.println("Left Terarium:");
    LeftBME.takeForcedMeasurement();
    print_values_serial(&LeftBME);
    lcd.setCursor(5, 1);
    print_values_LCD(&LeftBME, &lcd);

    Serial.print("Humid running: ");
    Serial.println(humidRunning);
  }

  EVERY_N_SECONDS(20) // send AIO values and update AIO status
  {
    send_aio_values(&RightBME, &LeftBME);
    lcd.setCursor(11, 2);
    lcd.print(io.status());
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
      lcd.setCursor(7, 3);
      lcd.print("Low...Running");
    }
    else if (humidRunning && humidReached)
    {
      humidRunning = false;
      digitalWrite(D0, 1);
      Serial.println("Turned OFF humidifier");
      lcd.setCursor(7, 3);
      lcd.print("OK           ");
    }
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

void print_values_LCD(Adafruit_BME280 *bmeSensor, LiquidCrystal_I2C *lcd)
{
  lcd->print("");
  lcd->print(bmeSensor->readTemperature());
  lcd->print("*C ");

  lcd->print(bmeSensor->readHumidity());
  lcd->print("%");
}

void send_aio_values(Adafruit_BME280 *insideSensor, Adafruit_BME280 *outsideSensor)
{
  teraInHum->save(insideSensor->readHumidity());
  teraInTemp->save(insideSensor->readTemperature());

  teraOuHum->save(outsideSensor->readHumidity());
  teraOutTemp->save(outsideSensor->readTemperature());
}