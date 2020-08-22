// 
// Pool Temperature Sensor for Weewx - for Wemos D1 Mini 
// 5/2020 - bubba@bubba.org
//
// Based on work by:
// Thermistor Example #3 from the Adafruit Learning System guide on Thermistors 
// 
// which analog pin to connect
#define THERMISTORPIN A0         
// resistance at 25 degrees C
#define THERMISTORNOMINAL  11000   // this depends on the temp sensor you buy
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 10
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950  // depends on the temp sensor you buy
// the value of the 'other' resistor
#define SERIESRESISTOR 9900  // depends on the measured resistance of your resistor

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h> // enable built-in Arduino OTA

// Uncomment the include below to enable another OTA library
// which will allow web-based uploads - also uncomment the
// webota.handle() call in loop function
// https://github.com/scottchiefbaker/ESP-WebOTA
// #include <WebOTA.h>  

const char* ssid = "SSID";
const char* pass = "PASSWORD";
const char* hostname = "PoolTemp"; // sent via DHCP request
float offset = 0;  // optional - you could also handle on the weewx side.

ESP8266WebServer server(80);
 
void setup(void) {
  Serial.begin(115200);
  delay(10);
  Serial.println("");
  Serial.print("Connecting to Wifi");
  //WiFi.hostname(hostname);
  wifi_station_set_hostname(hostname);
  WiFi.softAPdisconnect (true);
  WiFi.begin(ssid,pass);
  while(WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
  }
  Serial.println(".");
  Serial.print("Connected to SSID: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Hostname: ");
  Serial.println(hostname);

  //if (MDNS.begin(hostname)) {
  //  Serial.println("MDNS responder started.");
 // }
  
  server.on("/temp", handleTemp);
  server.onNotFound(handleNotFound);
  server.begin();
  
  ArduinoOTA.onStart([]() {  // handle OTA requests
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

float ntc () {
  uint8_t i;
  float average;
  int samples[NUMSAMPLES];

  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(THERMISTORPIN);
   delay(10);
  }
  
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  //Serial.print("Average analog reading "); 
  //Serial.println(average);
  
  // convert the value to resistance  1023 = Arduino adcMax analog to digital convert value returned by analogRead.
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  //Serial.print("Thermistor resistance "); 
  //Serial.println(average);
  
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  //steinhart = (steinhart * 9.0)/ 5.0 + 32.0;                  // Convert to Fahrenheit
  steinhart = steinhart*1.8 + 32.0;           
    
  return steinhart;
}

void handleTemp() {
  char oneDigit[5]; 
  float sensorValue = ntc();
  
  sensorValue += offset;
  
  String response = "{\"Temperature\":\"";
  
  dtostrf(sensorValue, 2, 1, oneDigit); // convert to 1 decimal place resolution

  response += oneDigit;
  response += "\"}";
  Serial.print("Temperature "); 
  Serial.print(oneDigit);
  Serial.println(" *F");
  server.send(200, "application/json", response);
}

// This method is called when an undefined url is specified by the caller
void handleNotFound() {
  server.send(400, "application/json", "{\"message\":\"Invalid request\"}");
}
 
void loop(void) {
  //webota.handle();
  ArduinoOTA.handle();
  server.handleClient();
}
