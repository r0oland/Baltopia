#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <Wire.h>
#include <AdafruitIO_WiFi.h>

#include "secrets.h"
#include "SHT21.h"

FASTLED_USING_NAMESPACE

void update_humid_values();
void setup_leds();
void pulse_leds(uint8_t nPulses, uint8_t pulseSpeed);
void setup_wifi();
void set_led_status(uint8_t status);

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
AdafruitIO_Feed *insideHumid = io.feed("inside_humid");
AdafruitIO_Feed *insideTemp = io.feed("inside_temp");

// read analog channels
uint16_t analogValue = 0;
uint8_t goodPins[13] = {34, 32, 33, 25, 26, 27, 14, 12, 13, 4, 0, 2, 15}; // they all seem to work...

SHT21 SHTSensor;

void setup()
{
  Serial.begin(921600); // pio device monitor -p COM11 -b 115200
  while(! Serial); // wait for serial monitor to open

  Serial.print("Setting up leds...");
  setup_leds();
  Serial.println("done!");
  set_led_status(1); // working

  // Serial.print("Setting up wifi connection:");
  // setup_wifi();

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(250);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  Serial.print("Setting up sensors...");
  Wire.begin(16, 17); // I2C_SDA = 16 | I2C_SCL = 17
  SHTSensor.begin();
  Serial.println("done!");

  Serial.println();
  Serial.println("Ready to rumble!");
  set_led_status(0); // all good
}

void loop()
{
  io.run();   // io.run(); is required for all sketches.
  // it should always be present at the top of your loop

  // analogValue = analogRead(goodPins[1]); // read first valid analog pin
  EVERY_N_SECONDS(5) { update_humid_values(); } // change patterns periodically

  if (io.status() >= AIO_CONNECTED)
    set_led_status(0); // all good
  else
    set_led_status(2); // error
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
  float humid = SHTSensor.getHumidity();
  Serial.print("Humidity(%RH): ");
  Serial.println(humid);
  insideHumid->save(humid);

  float temp = SHTSensor.getTemperature();
  Serial.print("Temperature(C): ");
  Serial.println(temp);
  insideTemp->save(temp);
  set_led_status(0); // all good
}

