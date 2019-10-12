#include "thingsboard_lib.h"
// define functions here

Iot::Iot(const char* newServer, const char* newToken, uint_fast32_t updateInt):tb(client)
{
  device_token = newToken;
  server = newServer;
  updateIOTInterval = updateInt;
};

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void Iot::send_data(float analTemp, float padTemp, float pwmValue){
  if ((previousDataSend == 0) || (millis() - previousDataSend >= updateIOTInterval)) {
    tb.sendTelemetryFloat("analTemp", analTemp);
    tb.sendTelemetryFloat("padTemp", padTemp);
    tb.sendTelemetryFloat("pwmValue", pwmValue);
    previousDataSend = millis();
  }
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// check connections to Wifi and IOT, reconnect if required
void Iot::check_connection() {
  if (WiFi.status() != WL_CONNECTED) {
    while (wifiStatus != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(WIFI_SSID);
      // Connect to WPA/WPA2 network
      wifiStatus = WiFi.begin(WIFI_SSID, WIFI_PASS);
      delay(5000);
    }
    Serial.println("Connected to AP");
  }

  if (!tb.connected()) {
    while (!tb.connected()) { // Loop until we're reconnected
      Serial.print("Connecting to ThingsBoard node ...");
      // Attempt to connect (clientId, username, password)
      // NOTE c_str() converts from String to const char*
      if (tb.connect(server.c_str(), device_token.c_str())){
        Serial.println( "[DONE]" );
      }
      else
      {
        Serial.print( "[FAILED]" );
        Serial.println( " : retrying in 5 seconds" );
        // Wait 5 seconds before retrying
        delay(5000);
      }
    }
  }
}
