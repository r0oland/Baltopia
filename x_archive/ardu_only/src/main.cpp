#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <SparkFunHTU21D.h>
#include <DHT.h>
#include <math.h>

void setupCurrentPath();
void sortMoistures();


// define digital pins
const uint8_t RELAIS_4 = 3; // fan?
const uint8_t RELAIS_3 = 4; // light
const uint8_t RELAIS_PUMP = 5; // pump
const uint8_t RELAIS_HEAT_PAD = 6; // heater
const uint8_t DHT_PIN = 8;

const uint8_t ONE_WIRE_TEMP_PIN = 7; // -127 = no pull down, not connected

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

DHT BoxDht(DHT_PIN, DHT22); //// Initialize DHT sensor for normal 16mhz Arduino

// Data wire is plugged into pin 2 on the Arduino
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire OneWireTempSensors(ONE_WIRE_TEMP_PIN);
/********************************************************************/
// Pass our OneWireTempSensors reference to Dallas Temperature.
DallasTemperature TempSensors(&OneWireTempSensors);

HTU21D RoomHumidTemp; // small sensor on the breadboard


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(RELAIS_HEAT_PAD, OUTPUT);
  pinMode(RELAIS_PUMP, OUTPUT);
  pinMode(RELAIS_3, OUTPUT);
  pinMode(RELAIS_4, OUTPUT);
  pinMode(MOIST_SUPPL_1, OUTPUT);
  pinMode(MOIST_SUPPL_2, OUTPUT);

  // relais is active low, so when high, it is not switched
  // turn of all relais for starters
  digitalWrite(RELAIS_HEAT_PAD, HIGH);
  digitalWrite(RELAIS_PUMP, HIGH);
  digitalWrite(RELAIS_3, HIGH);
  digitalWrite(RELAIS_4, HIGH);

  // start serial port

  // Start up the library
  TempSensors.begin();
  RoomHumidTemp.begin();
  BoxDht.begin();
  Serial.begin(9600);

  Serial.println("Basic Arduino ChilliGo");
}

// the loop function runs over and over again forever
void loop() {


  Serial.print("Found temperature sensors: ");
  Serial.println(TempSensors.getDeviceCount());

  Serial.print("Requesting all sensor values...");
  TempSensors.requestTemperatures(); // Send the command to get temperature readings

  float heatPadTemp = TempSensors.getTempCByIndex(0); // temp sensor heat pad
  float soilTemp = TempSensors.getTempCByIndex(1); // temp sensor heat pad

  float roomHumid = RoomHumidTemp.readHumidity();
  float roomTemp = RoomHumidTemp.readTemperature();

  float boxHumid = BoxDht.readHumidity();
  float boxTemp = BoxDht.readTemperature();

  float meanTemp = (soilTemp + boxTemp)/2;

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

  Serial.println("done!");

  // Print out all measure values

  Serial.print("Pad  Temperature: ");
  Serial.print(heatPadTemp);
  Serial.println(" C");

  Serial.print("Soil Temperature: ");
  Serial.print(soilTemp);
  Serial.println(" C");

  Serial.print("Room Temperature: ");
  Serial.print(roomTemp);
  Serial.println(" C");

  Serial.print("Box  Temperature: ");
  Serial.print(boxTemp);
  Serial.println(" C");

  Serial.print("Mean Temperature: ");
  Serial.print(meanTemp);
  Serial.println(" C");

  Serial.print("Room Humidity: ");
  Serial.print(roomHumid);
  Serial.println("%");

  Serial.print("Box  Humidity: ");
  Serial.print(boxHumid);
  Serial.println("%");

  // Print out median values
  Serial.print("sensor resistance = ");
  Serial.println(resistance);

  // set outputs (heat pad, pump) based on sensor values -----------------------

  // // FIXME add safety checks here!!!!
  // if ((heatPadTemp > 41.0) || (meanTemp > 27.0))
  // {
  //   digitalWrite(RELAIS_HEAT_PAD, HIGH); // high = off
  //   Serial.println("Heatpad is OFF");
  // }
  // else
  // {
  //   digitalWrite(RELAIS_HEAT_PAD, LOW); // low = off
  //   Serial.println("Heatpad is ON");
  // }

  // FIXME add safety checks here!!!!
  if (resistance > 3000)
  {
    Serial.print("Turning pump ON...");
    digitalWrite(RELAIS_PUMP, LOW); // high = off
    Serial.print("pumping...");
    delay(10000);
    digitalWrite(RELAIS_PUMP, HIGH); // high = off
    Serial.println("Done!");
  }
  else
  {
    Serial.println("Pump is OFF");
  }

  Serial.println("");
  delay(20000);
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
