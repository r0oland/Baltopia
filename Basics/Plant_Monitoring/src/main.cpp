#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
// #include <ESP8266WiFi.h>

#include <AdafruitIO_WiFi.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <DHT_U.h>
// #include <DHT.h>

#define DHTTYPE DHT22 // DHT 22 (AM2302)
#define DHTPIN_IN 5   // Digital pin connected to the DHT sensor
#define DHTPIN_OUT 18 // Digital pin connected to the DHT sensor

DHT_Unified DhtIn(DHTPIN_IN, DHTTYPE);
DHT_Unified DhtOut(DHTPIN_OUT, DHTTYPE);

#include "secrets.h"

FASTLED_USING_NAMESPACE

void send_aio_values();
void print_sensor_values();
void setup_leds();
void pulse_leds(uint8_t nPulses, uint8_t pulseSpeed);
void setup_wifi();
void set_led_status(uint8_t status);
int32_t measure_resistance(uint8_t digitalDrivePin, uint8_t digitalCounterPin, uint8_t analogReadPin);
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
AdafruitIO_Feed *soilMoist1 = io.feed("soil_moisture_1_new");
AdafruitIO_Feed *soilMoist2 = io.feed("soil_moisture_2_new");

// read analog channels
// defines for moisture sensins
uint16_t analogValue = 0;
// uint8_t goodPins[13] = {34, 32, 33, 35}; // these work with wifi...
const uint8_t MOIST_SENSE_1 = 33;
const uint8_t MOIST_DRIVE_1 = 12;
const uint8_t MOIST_COUNTER_1 = 13;
const uint8_t MOIST_SENSE_2 = 34;
const uint8_t MOIST_DRIVE_2 = 14;
const uint8_t MOIST_COUNTER_2 = 27;
const uint8_t N_READS = 5;           // Number of sensor reads for filtering
const uint16_t COMP_RESISTOR = 6500; // Constant value of known resistor in Ohms
const uint32_t MEGA = 1000000;
const uint16_t MAX_READ_OUT = 3055; // analog read after diode with 3.3V

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
  pinMode(MOIST_DRIVE_1, OUTPUT);
  pinMode(MOIST_COUNTER_1, OUTPUT);
  pinMode(MOIST_DRIVE_2, OUTPUT);
  pinMode(MOIST_COUNTER_2, OUTPUT);

  Serial.println();
  Serial.println("Ready to rumble!");
  set_led_status(0); // all good
  send_aio_values();
}

void loop()
{
  io.run(); // io.run(); is required for all sketches.
  // it should always be present at the top of your loop

  // analogValue = analogRead(goodPins[1]); // read first valid analog pin
  // uint32_t currentRes1 = measure_resistance(MOIST_DRIVE_1, MOIST_COUNTER_1, MOIST_SENSE_1);
  // Serial.print("Measured resistance 1: ");
  // Serial.println(currentRes1);

  EVERY_N_SECONDS(20) { send_aio_values(); }    // change patterns periodically
  EVERY_N_SECONDS(5) { print_sensor_values(); } // change patterns periodically

  if (io.status() >= AIO_CONNECTED)
    set_led_status(0); // all good
  else
    set_led_status(2); // error
  delay(100);
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

void send_aio_values()
{
  set_led_status(1); // working
  sensors_event_t event;
  // get inside temperature ----------------------------------------------------
  DhtIn.temperature().getEvent(&event);
  if (isnan(event.temperature))
  {
    Serial.println(F("Error reading temperature!"));
    set_led_status(2); // red == error
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
    set_led_status(2); // red == error
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
    set_led_status(2); // red == error
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
    set_led_status(2); // red == error
  }
  else
  {
    Serial.print(F("  Humidity Outside: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
    outsideHumid->save(event.relative_humidity);
  }

  // measure soil moistures
  uint32_t currentRes1 = measure_resistance(MOIST_DRIVE_1, MOIST_COUNTER_1, MOIST_SENSE_1);
  Serial.print(F("  soil moisture 1: "));
  Serial.print(currentRes1);
  Serial.println(F(" Ohm"));
  soilMoist1->save(currentRes1);

  uint32_t currentRes2 = measure_resistance(MOIST_DRIVE_2, MOIST_COUNTER_2, MOIST_SENSE_2);
  Serial.print(F("  soil moisture 2: "));
  Serial.print(currentRes2);
  Serial.println(F(" Ohm"));
  soilMoist2->save(currentRes2);

  set_led_status(0); // all good
}

void print_sensor_values()
{
  set_led_status(1); // working
  sensors_event_t event;
  // get inside temperature ----------------------------------------------------
  DhtIn.temperature().getEvent(&event);
  Serial.print(F("Temperature Inside: "));
  Serial.print(event.temperature);
  Serial.print(F("°C"));

  // get inside humidity ----------------------------------------------------
  DhtIn.humidity().getEvent(&event);
  Serial.print(F("  Humidity Inside: "));
  Serial.print(event.relative_humidity);
  Serial.println(F("%"));

  // get outside temperature ----------------------------------------------------
  DhtOut.temperature().getEvent(&event);
  Serial.print(F("Temperature Outside: "));
  Serial.print(event.temperature);
  Serial.print(F("°C"));

  // get outside humdidity ----------------------------------------------------
  DhtOut.humidity().getEvent(&event);
  Serial.print(F("  Humidity Outside: "));
  Serial.print(event.relative_humidity);
  Serial.println(F("%"));

  // measure soil moistures
  uint32_t currentRes1 = measure_resistance(MOIST_DRIVE_1, MOIST_COUNTER_1, MOIST_SENSE_1);
  Serial.print(F("  soil moisture 1: "));
  Serial.print(currentRes1);
  Serial.println(F(" Ohm"));

  uint32_t currentRes2 = measure_resistance(MOIST_DRIVE_2, MOIST_COUNTER_2, MOIST_SENSE_2);
  Serial.print(F("  soil moisture 2: "));
  Serial.print(currentRes2);
  Serial.println(F(" Ohm"));

  set_led_status(0); // all good
}

int32_t measure_resistance(uint8_t digitalDrivePin, uint8_t digitalCounterPin, uint8_t analogReadPin)
{
  // digitalDrivePin - set this high/low to actually make current flow
  // digitalCounterPin - set this high/low to make reverse current flow,
  // otherwise we might do electrolysis or some shit
  // analogReadPin - read from this pin to get the actual value
  int32_t resistance = 0;     // reset resistance
  int32_t tempResistance = 0; // reset resistance

  // measure analog values N times, switch between current paths each time
  for (int i = 0; i < N_READS; i++)
  {
    // setupCurrentPath(currentPath, 1); // Prepare the digital and analog pin values

    digitalWrite(digitalDrivePin, HIGH); // set the voltage supply on
    delay(10);
    int16_t sensorReadOut = analogRead(analogReadPin); // read the sensor voltage
    digitalWrite(digitalDrivePin, LOW);                // set the voltage supply off
    delay(10);

    // counter the read current, but don't measure, as we don't want to waste
    // the few precios analog pins we have...
    digitalWrite(digitalCounterPin, HIGH); // set the voltage supply on
    delay(10);
    digitalWrite(digitalCounterPin, LOW); // set the voltage supply off
    delay(10);

    if ((sensorReadOut >= MAX_READ_OUT) || ((MAX_READ_OUT - sensorReadOut) <= 10))
    {
      tempResistance = 1 * MEGA; // upper limit for our resistance
    }
    else
      tempResistance = int32_t(float(COMP_RESISTOR) * (MAX_READ_OUT - sensorReadOut) / sensorReadOut);

    resistance += tempResistance;
  }

  resistance = resistance / N_READS;
  if (resistance > MEGA)
    resistance = MEGA;

  return resistance;
}
