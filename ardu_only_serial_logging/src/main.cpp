#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>

void setupCurrentPath();
void sortMoistures();

uint32_t totalTime =0;

// define digital pins
const uint8_t RELAIS_4 = 3; // fan?
const uint8_t RELAIS_3 = 4; // light
const uint8_t RELAIS_2 = 5; // pump
const uint8_t RELAIS_HEAT_PAD = 6; // heater

const uint8_t TEMP_HEAT_PAD = 7; // -127 = no pull down, not connected

const uint8_t MOIST_SUPPL_1 = 12; // -127 = no pull down, not connected
const uint8_t MOIST_SUPPL_2 = 13; // -127 = no pull down, not connected

// define my constants
const uint8_t N_READS = 10;    // Number of sensor reads for filtering
const uint16_t COMP_RESISTOR = 1500;  // Constant value of known resistor in Ohms

// define other stuff...........
int32_t resistance;

int activeDigitalPin = 6;         // 6 or 7 interchangeably
int supplyVoltageAnalogPin;       // 6-ON: A0, 7-ON: A1
int sensorVoltageAnalogPin;       // 6-ON: A1, 7-ON: A0

int supplyVoltage;                // Measured supply voltage
int sensorVoltage;                // Measured sensor voltage

int i;                            // Simple index variable

// Data wire is plugged into pin 2 on the Arduino
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire TempHeatPad(TEMP_HEAT_PAD);
/********************************************************************/
// Pass our TempHeatPad reference to Dallas Temperature.
DallasTemperature Sensors(&TempHeatPad);

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(RELAIS_HEAT_PAD, OUTPUT);
  pinMode(RELAIS_2, OUTPUT);
  pinMode(RELAIS_3, OUTPUT);
  pinMode(RELAIS_4, OUTPUT);
  pinMode(MOIST_SUPPL_1, OUTPUT);
  pinMode(MOIST_SUPPL_2, OUTPUT);

  // relais is active low, so when high, it is not switched
  // turn of all relais for starters
  digitalWrite(RELAIS_HEAT_PAD, HIGH);
  digitalWrite(RELAIS_2, HIGH);
  digitalWrite(RELAIS_3, HIGH);
  digitalWrite(RELAIS_4, HIGH);

  // start serial port
  Serial.begin(9600);
  // Start up the library
  Sensors.begin();
}

// the loop function runs over and over again forever
void loop() {

  float heatPadTemp = Sensors.getTempCByIndex(0); // temp sensor heat pad

  float soilTemp = Sensors.getTempCByIndex(1); // temp sensor heat pad

  // FIXME add safety checks here!!!!
  if ((heatPadTemp > 50.0) || (soilTemp > 30.0))
  {
    digitalWrite(RELAIS_HEAT_PAD, HIGH); // high = off
    Serial.println("0");
  }
  else
  {
    digitalWrite(RELAIS_HEAT_PAD, LOW); // low = off
    Serial.println("1");
  }

  // measure the soil moisture -------------------------------------------------
  // read sensor, filter, and calculate resistance value
  // Noise filter: median filter
  resistance = 0; // reset resistance
  for (i=0; i < N_READS; i++) {

    setupCurrentPath();      // Prepare the digital and analog pin values

    // Read 1 pair of voltage values
    digitalWrite(activeDigitalPin, HIGH);                 // set the voltage supply on
    delay(10);
    supplyVoltage = analogRead(supplyVoltageAnalogPin);   // read the supply voltage
    sensorVoltage = analogRead(sensorVoltageAnalogPin);   // read the sensor voltage
    digitalWrite(activeDigitalPin, LOW);                  // set the voltage supply off
    delay(10);


    // Calculate resistance
    // the 0.5 add-term is used to round to the nearest integer
    resistance += int32_t(float(COMP_RESISTOR)*(supplyVoltage-sensorVoltage)/sensorVoltage+0.5);
  }
  resistance = int32_t(resistance/N_READS); // calculate mean
  totalTime = uint32_t(millis()/1000);

  // Print out median values
  Serial.println(totalTime);
  Serial.println(resistance);
  Serial.println(heatPadTemp);
  Serial.println(soilTemp);


  delay(10000);
}

void setupCurrentPath() {
  if ( activeDigitalPin == MOIST_SUPPL_1 ) {
    activeDigitalPin = MOIST_SUPPL_2;
    supplyVoltageAnalogPin = A1;
    sensorVoltageAnalogPin = A0;
  }
  else {
    activeDigitalPin = MOIST_SUPPL_1;
    supplyVoltageAnalogPin = A0;
    sensorVoltageAnalogPin = A1;
  }
}
