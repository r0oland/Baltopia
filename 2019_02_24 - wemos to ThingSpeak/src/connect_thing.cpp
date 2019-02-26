/*
 ESP8266 --> ThingSpeak Channel

 This sketch sends the value of Analog Input (A0) to a ThingSpeak channel
 using the ThingSpeak API (https://www.mathworks.com/help/thingspeak).

 Requirements:

   * ESP8266 Wi-Fi Device
   * Arduino 1.8.8+ IDE
   * Additional Boards URL: http://arduino.esp8266.com/stable/package_esp8266com_index.json
   * Library: esp8266 by ESP8266 Community
   * Library: ThingSpeak by MathWorks

 ThingSpeak Setup:

   * Sign Up for New User Account - https://thingspeak.com/users/sign_up
   * Create a new Channel by selecting Channels, My Channels, and then New Channel
   * Enable one field
   * Enter SECRET_CH_ID in "secrets.h"
   * Enter SECRET_WRITE_APIKEY in "secrets.h"

 Setup Wi-Fi:
  * Enter SECRET_SSID in "secrets.h"
  * Enter SECRET_PASS in "secrets.h"

 Tutorial: http://nothans.com/measure-wi-fi-signal-levels-with-the-esp8266-and-thingspeak

 Created: Feb 3, 2017 by Hans Scharler (http://nothans.com)
*/

#include "ThingSpeak.h"
#include "secrets.h"

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

const int GREEN_LED = 5;
const int RED_LED = 4;

#include <ESP8266WiFi.h>

char ssid[] = SECRET_SSID;   // your network SSID (name)
char pass[] = SECRET_PASS;   // your network password
int keyIndex = 0;            // your network key index number (needed only for WEP)
WiFiClient  client;

void setup() {

  pinMode(GREEN_LED, OUTPUT);
  // pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  // digitalWrite(RED_LED, HIGH);   // turn the LED on (HIGH is the voltage level)

  // Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_STA);

  ThingSpeak.begin(client);
}

void loop() {

  // Connect or reconnect to WiFi
  if (WiFi.status() != WL_CONNECTED) {
    // Serial.print("Attempting to connect to SSID: ");
    // Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      // Serial.print(".");
      delay(5000);
    }
    // Serial.println("\nConnected.");
  }

  // digitalWrite(RED_LED, HIGH);    // turn the LED off by making the voltage HIGH

  // Measure Analog Input (A0)
  int valueA0 = analogRead(A0);

  // Write value to Field 1 of a ThingSpeak Channel
  int httpCode = ThingSpeak.writeField(myChannelNumber, 1, valueA0, myWriteAPIKey);

  if (httpCode == 200) {
    // Serial.println("Channel write successful.");
    digitalWrite(GREEN_LED, HIGH);    // turn the LED off by making the voltage LOW
    // digitalWrite(RED_LED, LOW);    // turn the LED off by making the voltage LOW
  }
  else {
    digitalWrite(GREEN_LED, LOW);    // turn the LED off by making the voltage LOW
    // digitalWrite(RED_LED, HIGH);    // turn the LED off by making the voltage LOW
    // Serial.println("Problem writing to channel. HTTP error code " + String(httpCode));
  }

  // Wait 20 seconds to uodate the channel again
  delay(10000*6);
  digitalWrite(GREEN_LED, LOW);    // turn the LED off by making the voltage LOW
  // digitalWrite(RED_LED, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);
  digitalWrite(GREEN_LED, HIGH);    // turn the LED off by making the voltage HIGH
  delay(1000);
  digitalWrite(GREEN_LED, LOW);    // turn the LED off by making the voltage LOW


}
