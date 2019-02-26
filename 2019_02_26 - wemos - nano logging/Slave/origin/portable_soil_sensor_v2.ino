
// http://vanderleevineyard.com/vineyard-blog/vinduino-20-soil-moisture-sensor-software-update
 
// "Vinduino" portable soil moisture sensor code V2.0
// Date November 12, 2012
// Reinier van der Lee
// www.vanderleevineyard.com

// include the library code only for LCD display version
#include <LiquidCrystal.h>
#include <math.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// define Ohm character
byte ohm[8] = {
  B00000,
  B01110,
  B10001,
  B10001,
  B10001,
  B01010,
  B11011,
  B00000,
};

// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin1 = A0;  // Analog input pin that the gypsum sensor is attached to
const int analogInPin2 = A1;  // Analog input pin that the gypsum sensor is attached to


int NUM_READS = 10;
int sensorvalue3 = 0;        // value read from gypsum sensor
int sensorvalue4 = 0;        // value read from sensor supply
int sensorvoltage2 = 0;
int supplyvoltage2 = 0;
int unsigned long Rsense1 = 0;
long Rsense2 = 0;
int unsigned long avsensorvalue3 = 0;
int unsigned long avsensorvalue4 = 0;
int B = 0;
int unsigned long moisture_pct = 0;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

  // set up the LCD's number of columns and rows:
  lcd.createChar(1, ohm);
  lcd.begin(16, 2);
  lcd.print ("Vinduino V2.0");


    // initialize the digital pin as an output.
  // Pin 6 is sense resistor voltage supply 1
  pinMode(6, OUTPUT);

    // initialize the digital pin as an output.
  // Pin 7 is sense resistor voltage supply 2
  pinMode(7, OUTPUT);

     // initialize the digital pin as an output.
  // Pin 8 is sense resistor voltage supply 2
  pinMode(8, OUTPUT);

    // initialize the digital pin as an output.
  // Pin 9 is sense resistor voltage supply 2
  pinMode(9, OUTPUT);

    delay(500);
}

void loop() {

// read sensor, average, and calculate resistance value

    // Noise filter
  // averaging filter

  avsensorvalue3 = 0;
  avsensorvalue4 = 0;

for (B=0; B< NUM_READS; B++) {

 // Read input 1 values

    digitalWrite(7, HIGH);   // set the voltage supply on

      delay(10);

  // read the analog in value:
  sensorvalue4 = analogRead(analogInPin2);

    //delay(100);

  digitalWrite(7, LOW);    // set the voltage supply off

   delay(10);

  digitalWrite(6, HIGH);   // set the voltage supply on

    delay(10);

  // read the analog in value:
  sensorvalue3 = analogRead(analogInPin2);

  digitalWrite(6, LOW);    // set the voltage supply off


  avsensorvalue4 = avsensorvalue4 + sensorvalue4;
  avsensorvalue3 = avsensorvalue3 + sensorvalue3;

  }

  // end of multiple read loop

  // average all samples out

  avsensorvalue4 = avsensorvalue4 / NUM_READS;
  avsensorvalue3 = avsensorvalue3 / NUM_READS;


  float sensorvoltage2 = avsensorvalue3 * 4.88 / 1023.0;

  float supplyvoltage2 = avsensorvalue4 * 4.88 / 1023.0;
  //     if (sensorvalue3 < 10)
  //     {
  //      Rsense2= 0;
  //      }
  //      else
  //      {
      Rsense2 = (1200 / sensorvoltage2) * (supplyvoltage2 - sensorvoltage2);
  //      }

moisture_pct = pow(float(Rsense2/31.65),float (1/-1.695))*400;
if (moisture_pct > 100)
{
  moisture_pct =100;
}
else
{moisture_pct = moisture_pct;
}

  Serial.print("sensor resistance = ");
  Serial.println(Rsense2);

  // set the cursor to column 0, line 0
  lcd.setCursor(0, 0);

  //Clear the LCD
  lcd.print("sensor =        ");
  lcd.setCursor(0, 1);
  lcd.print("moisture:        ");

  // set the cursor to column 0, line 0
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(moisture_pct);
  lcd.print(" %");

  // set the cursor to column 0, line 1
  lcd.setCursor(0, 1);
  // Print a message to the LCD.
  lcd.print("Sensor: " );
  lcd.print(Rsense2);
  lcd.print(" ");
  lcd.write(1);


  // delay until next measurement (msec)

  delay(5000);

}
