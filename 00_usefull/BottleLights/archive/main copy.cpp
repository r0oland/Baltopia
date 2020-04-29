#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <AdafruitIO_WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTTYPE DHT22 // DHT 22 (AM2302)
#define DHTPIN_IN 5   // Digital pin connected to the DHT sensor
#define DHTPIN_OUT 18 // Digital pin connected to the DHT sensor

DHT_Unified DhtIn(DHTPIN_IN, DHTTYPE);
DHT_Unified DhtOut(DHTPIN_OUT, DHTTYPE);

#include "secrets.h"

FASTLED_USING_NAMESPACE

void update_humid_values();
void setup_leds();
void pulse_leds(uint8_t nPulses, uint8_t pulseSpeed);
void setup_wifi();
void set_led_status(uint8_t status);
int32_t measure_resistance(uint8_t currentPath);
void setupCurrentPath(uint8_t iPath, uint8_t changeCurrentPath);

// defin LED parameters
#define DATA_PIN 15
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 4
#define BRIGHTNESS 25
#define FRAMES_PER_SECOND 30
CRGB leds[NUM_LEDS]; // contains led info, this we set first, the we call led show

// setup adafruit stuff
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PWD);
AdafruitIO_Feed *insideHumid = io.feed("inside_humid2");
AdafruitIO_Feed *insideTemp = io.feed("inside_temp2");
AdafruitIO_Feed *outsideTemp = io.feed("outside_temp2");
AdafruitIO_Feed *outsideHumid = io.feed("outside_humid2");

// read analog channels
// defines for moisture sensins
uint16_t analogValue = 0;
uint8_t goodPins[13] = {34, 32, 33, 25, 26, 27, 14, 12, 13, 4, 0, 2, 15}; // they all seem to work...
const uint8_t MOIST_SENSE_1 = 12;
const uint8_t MOIST_SENSE_2 = 13;
const uint8_t MOIST_SUPPL_1 = 17;
const uint8_t MOIST_SUPPL_2 = 16;
uint32_t fullRes1;
uint8_t activeDigitalPin;
uint8_t doReadForward = 0;           // switch between current during analog read of moisture sensor
const uint8_t N_READS = 10;          // Number of sensor reads for filtering
const uint16_t COMP_RESISTOR = 6000; // Constant value of known resistor in Ohms
const uint32_t MEGA = 1000000;
uint8_t supplyVoltageAnalogPin;
uint8_t sensorVoltageAnalogPin;

void setup()
{
  Serial.begin(921600); // pio device monitor -p COM11 -b 115200
  while (!Serial)
    ; // wait for serial monitor to open

  Serial.print("Setting up leds...");
  setup_leds();
  Serial.println("done!");
  set_led_status(1); // working

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

  Serial.print("Setting up sensors...");
  DhtIn.begin();
  DhtOut.begin();
  Serial.println("done!");

  // setup pins
  pinMode(MOIST_SUPPL_1, OUTPUT);
  pinMode(MOIST_SUPPL_2, OUTPUT);
  fullRes1 = measure_resistance(0);

  Serial.println();
  Serial.println("Ready to rumble!");
  set_led_status(0); // all good
}

void loop()
{
  io.run(); // io.run(); is required for all sketches.
  // it should always be present at the top of your loop

  // analogValue = analogRead(goodPins[1]); // read first valid analog pin
  uint32_t currentRes1 = measure_resistance(0);
  fullRes1 = (currentRes1 + fullRes1) / 2; // moving average
  Serial.print("[SLAVE] Measured resistance 1: ");
  Serial.println(currentRes1);
  Serial.print("[SLAVE] Averaged resistance 1: ");
  Serial.println(fullRes1);

  EVERY_N_SECONDS(20) { update_humid_values(); } // change patterns periodically

  if (io.status() >= AIO_CONNECTED)
    set_led_status(0); // all good
  else
    set_led_status(2); // error
  delay(500);
}

void setup_leds()
{
  // tell FastLED about the LED strip configuration
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  FastLED.clear();
  FastLED.show();
  for (int led = 0; led < NUM_LEDS; led++)
  {
    leds[led] = CRGB::White;
    delay(200);
    FastLED.show();
  }
  delay(200);
  pulse_leds(3, 5);
  FastLED.clear();
  FastLED.show();
}

void pulse_leds(uint8_t nPulses, uint8_t pulseSpeed)
{
  uint8_t ledFade = 255;      // start with LEDs off
  int8_t additionalFade = -5; // start with LEDs getting brighter
  uint8_t iPulse = 0;

  while (iPulse < nPulses)
  {
    for (uint8_t iLed = 0; iLed < NUM_LEDS; iLed++)
    {
      leds[iLed].setRGB(255, 255, 255);
      leds[iLed].fadeLightBy(ledFade);
    }
    FastLED.show();
    ledFade = ledFade + additionalFade;
    // reverse the direction of the fading at the ends of the fade:
    if (ledFade == 0 || ledFade == 255)
      additionalFade = -additionalFade;
    if (ledFade == 255)
      iPulse++;
    delay(pulseSpeed); // This delay sets speed of the fade. I usually do from 5-75 but you can always go higher.
  }
}

void set_led_status(uint8_t status)
{
  // first led in array displays overall status
  // (0 = all good, 1 = working, 2 = error)
  switch (status)
  {
  case 0:
    leds[0].setRGB(0, 255, 0); // all good = green
    FastLED.show();
    break;
  case 1:
    leds[0].setRGB(200, 165, 0); // working == orange
    FastLED.show();
    break;
  case 2:
    leds[0].setRGB(255, 0, 0); // error == red
    FastLED.show();
    break;

  default:
    leds[0].setRGB(255, 0, 0);
    FastLED.show();
    break;
  }
}

void setup_wifi()
{
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void update_humid_values()
{
  set_led_status(1); // working
  sensors_event_t event;
  // get inside temperature ----------------------------------------------------
  DhtIn.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    Serial.println(F("Error reading temperature!"));
    set_led_status(2); // all good
  }
  else
  {
    Serial.print(F("Temperature Inside: "));
    Serial.print(event.temperature);
    Serial.print(F("°C"));
    insideTemp->save(event.temperature);
  }

  // get inside humidity ----------------------------------------------------
  DhtIn.humidity().getEvent(&event);
  if (isnan(event.relative_humidity))
  {
    Serial.println(F("Error reading humidity!"));
    set_led_status(2); // all good
  }
  else
  {
    Serial.print(F("  Humidity Inside: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    insideHumid->save(event.relative_humidity);
  }

  // get outside temperature ----------------------------------------------------
  DhtOut.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    Serial.println(F("Error reading temperature!"));
    set_led_status(2); // all good
  }
  else
  {
    Serial.print(F("Temperature Outside: "));
    Serial.print(event.temperature);
    Serial.print(F("°C"));
    outsideTemp->save(event.temperature);
  }

  // get outside humdidity ----------------------------------------------------
  DhtOut.humidity().getEvent(&event);
  if (isnan(event.relative_humidity))
  {
    Serial.println(F("Error reading humidity!"));
    set_led_status(2); // all good
  }
  else
  {
    Serial.print(F("  Humidity Outside: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    outsideHumid->save(event.relative_humidity);
    set_led_status(0); // all good
  }
}

int32_t measure_resistance(uint8_t currentPath)
{
  // Serial.print("[SLAVE] Measuring soil resistance...");
  // http://vanderleevineyard.com/vineyard-blog/-the-vinduino-project-3-make-a-low-cost-soil-moisture-sensor-reader
  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter

  int32_t resistance = 0;     // reset resistance
  int32_t tempResistance = 0; // reset resistance
  for (int i = 0; i < N_READS; i++)
  {
    setupCurrentPath(currentPath, 1); // Prepare the digital and analog pin values

    digitalWrite(activeDigitalPin, HIGH); // set the voltage supply on
    delay(10);
    int16_t supplyReadOut = analogRead(26); // read the supply voltage
    int16_t sensorReadOut = analogRead(25); // read the sensor voltage
    digitalWrite(activeDigitalPin, LOW);    // set the voltage supply off
    delay(10);
    Serial.print("supplyReadOut");
    Serial.println(supplyReadOut);
    Serial.print("sensorReadOut");
    Serial.println(sensorReadOut);

    int16_t diffRead = supplyReadOut - sensorReadOut;
    // make sure we don't have short, in which case both values are very close
    if ((sensorReadOut >= 3) & (diffRead > 3))
      tempResistance = int32_t(float(COMP_RESISTOR) * (supplyReadOut - sensorReadOut) / sensorReadOut);
    else if ((sensorReadOut >= 3) & !(diffRead > 3)) // short, so set res close to zero
      tempResistance = 1;                            // set to 1M ohm as upper limit
    else
      tempResistance = MEGA * 3; // set to 1M ohm as upper limit
                                 // if sensor readout becomes too small, resistance values become meaningless
                                 // so set upper limit for possible resistance
    resistance += tempResistance;
  }

  resistance = resistance / N_READS;
  if (resistance > MEGA * 3)
    resistance = MEGA * 3; // set to mega ohm

  return resistance;
}

void setupCurrentPath(uint8_t iPath, uint8_t changeCurrentPath)
{
  // setup current paths for nPaths, where each path is connected to their own
  // moisture sensing gypsum sensor
  switch (iPath)
  {
  case 0: // first current path
    if (doReadForward)
    {
      activeDigitalPin = MOIST_SUPPL_2;
      supplyVoltageAnalogPin = MOIST_SENSE_2;
      sensorVoltageAnalogPin = MOIST_SENSE_1;
    }
    else
    {
      activeDigitalPin = MOIST_SUPPL_1;
      supplyVoltageAnalogPin = MOIST_SENSE_1;
      sensorVoltageAnalogPin = MOIST_SENSE_2;
    }
    break;
  default:
    break;
  }

  if (changeCurrentPath)
    doReadForward != doReadForward;
}