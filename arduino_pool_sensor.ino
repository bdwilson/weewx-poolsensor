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

WiFiServer server(80);
 
void setup(void) {
  Serial.begin(115200);
  //analogReference(EXTERNAL);  // Wemos D1 doesn't have a analog ref
  delay(10);
  WiFi.hostname(hostname);
  WiFi.begin(ssid,pass);
  while(WiFi.status() != WL_CONNECTED) {
     delay(500);
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.begin(); // start webserver

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
 
  // convert the value to resistance  1023 = Arduino adcMax analog to digital convert value returned by analogRead.
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  // comment out the below line for Celsius
  steinhart = steinhart*1.8 + 32.0;            // Convert to Fahrenheit
    
  return steinhart;
}
 
void loop(void) {
  char oneDigit[5]; 
  
  webota.handle();
  ArduinoOTA.handle();

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  while(!client.available()){
    delay(1);
  }

  float sensorValue = ntc();
  dtostrf(sensorValue, 2, 1, oneDigit); // convert to 1 decimal place resolution
  Serial.print("Temperature "); 
  Serial.print(oneDigit);
  Serial.println(" *F"); // change to C if you don't convert to F.
  client.print("{\"Temperature\":\"");
  client.print(oneDigit);
  client.print("\"}");

  delay(1);
}
