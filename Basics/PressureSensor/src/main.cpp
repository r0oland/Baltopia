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

Adafruit_BME280 InsideBME;  // I2C
Adafruit_BME280 OutsideBME; // I2C

uint16_t delayTime = 1000;
const uint8_t INSIDE_ADDRESS = 0x77;
const uint8_t OUTSIDE_ADDRESS = 0x76;

// setup adafruit stuff
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PWD);
AdafruitIO_Feed *teraInHum = io.feed("teraInHum");
AdafruitIO_Feed *teraOuHum = io.feed("teraOuHum");
AdafruitIO_Feed *teraInTemp = io.feed("teraInTemp");
AdafruitIO_Feed *teraOutTemp = io.feed("teraOutTemp");

void setup()
{
  Serial.begin(921600);
  while (!Serial)
    ; // wait for serial monitor to open

  lcd.init(); // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hallo Geliebte!");
  lcd.setCursor(0, 1);
  lcd.print("Ich lieb dich enton!");
  lcd.setCursor(0, 2);
  lcd.print("3000+");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" In: ");
  lcd.setCursor(0, 1);
  lcd.print("Out: ");
  lcd.setCursor(0, 2);
  lcd.print("IO status: ");

  Serial.print(F("Looking for inside humid. sensor..."));
  if (!InsideBME.begin(INSIDE_ADDRESS, &Wire))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
  Serial.println("found!");

  Serial.print(F("Looking for outside humid. sensor..."));
  if (!OutsideBME.begin(OUTSIDE_ADDRESS, &Wire))
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
  Serial.println("found!");

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // wait for a connection
  while (io.status() < AIO_CONNECTED)
  {
    Serial.print(".");
    delay(250);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  send_aio_values(&InsideBME, &OutsideBME);
}

void loop()
{
  io.run(); // io.run(); is required for all sketches.

  EVERY_N_SECONDS(1)
  {
    // Only needed in forced mode! In normal mode, you can remove the next line.
    InsideBME.takeForcedMeasurement(); // has no effect in normal mode
    Serial.println("Inside Terarium:");
    print_values_serial(&InsideBME);
    lcd.setCursor(5, 0);
    print_values_LCD(&InsideBME, &lcd);

    OutsideBME.takeForcedMeasurement(); // has no effect in normal mode
    Serial.println("Outside Terarium:");
    print_values_serial(&OutsideBME);
    lcd.setCursor(5, 1);
    print_values_LCD(&OutsideBME, &lcd);
  }

  EVERY_N_SECONDS(20)
  {
    send_aio_values(&InsideBME, &OutsideBME);
    lcd.setCursor(11, 2);
    lcd.print(io.status());
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