#include <Arduino.h>
#include <FastLED.h>

void setup_leds();
void pulse_leds(uint8_t nPulses, uint8_t pulseSpeed);
void Fire2012WithPalette();

FASTLED_USING_NAMESPACE

// defin LED parameters
#define COOLING 55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120

#define DATA_PIN D1
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 55
#define BRIGHTNESS 255
#define FRAMES_PER_SECOND 30
    CRGB leds[NUM_LEDS]; // contains led info, this we set first, the we call led show

bool gReverseDirection = false;
CRGBPalette16 gPal;

void setup()
{
  Serial.begin(921600); // pio device monitor -p COM11 -b 115200
  while (!Serial)
    ; // wait for serial monitor to open

  Serial.print("Setting up leds...");
  setup_leds();
  Serial.println("done!");

  gPal = HeatColors_p;
}

void loop()
{
  // Serial.print("Clearing leds...");
  // FastLED.clear();
  // FastLED.show();
  // for (int led = 0; led < NUM_LEDS; led++)
  // {
  //   leds[led] = CRGB::White;
  //   delay(20);
  //   FastLED.show();
  // }
  // delay(200);
  // pulse_leds(3, 5);
  // FastLED.clear();
  // FastLED.show();

  // random16_add_entropy(random());
  Fire2012WithPalette(); // run simulation frame, using palette colors
  FastLED.show();        // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
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
    delay(20);
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


void Fire2012WithPalette()
{
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for (int i = 0; i < NUM_LEDS; i++)
  {
    heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for (int k = NUM_LEDS - 1; k >= 2; k--)
  {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if (random8() < SPARKING)
  {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // Step 4.  Map from heat cells to LED colors
  for (int j = 0; j < NUM_LEDS; j++)
  {
    // Scale the heat value from 0-255 down to 0-240
    // for best results with color palettes.
    byte colorindex = scale8(heat[j], 240);
    CRGB color = ColorFromPalette(gPal, colorindex);
    int pixelnumber;
    if (gReverseDirection)
    {
      pixelnumber = (NUM_LEDS - 1) - j;
    }
    else
    {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}
