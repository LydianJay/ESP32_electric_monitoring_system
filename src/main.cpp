#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h> 
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>


#define RXD2 16
#define TXD2 17

constexpr uint8_t pinLED = 19;



const String serverIP         = "eleksi.lyncxus.site";
const char *ntpServer         = "time.windows.com";
const long gmtOffset_sec      = 8 * 3600; // e.g. for Philippines (GMT+8)
const int daylightOffset_sec  = 0;
// float energyOffset            = 11.262; // I have accidentally rest energy level so we need to add this.


uint64_t time1                = 0;
uint64_t time2                = 0;
constexpr uint8_t tickTime    = 55; //Every 55 seconds instead of 60 to accomodate some delays
uint64_t elapseTime           = 0;

struct PZEMReading {
  float voltage;
  float power;
  float current;
  float energy;
};

PZEMReading reading;
PZEM004Tv30 pzem(Serial2, RXD2, TXD2);

void getReadings();

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  pinMode(pinLED, OUTPUT);
  pinMode(14, OUTPUT); digitalWrite(14, LOW);
  digitalWrite(pinLED, HIGH);
  WiFiManager wm;
  if(!wm.autoConnect("eleksi", "admin123")) {
    digitalWrite(pinLED, HIGH);
  }
  
  Serial.println("Connecting...");
  
  digitalWrite(pinLED, LOW);
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
  time1 = micros();
  time2 = micros();
  getReadings();
}

void loop() {
  time1 = micros();

  if(time1 - time2 > tickTime * 1000000) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      int day = timeinfo.tm_mday;
      if (day == 1) // Need to reset the energy reading every month
      {
        // Reset the energy reading in the first day of the month 
        // Device should run in the first day or it will not reset
        Serial.println("Today is the first day of the month! Reset ENERGY");
        // energyOffset = 0;
        // pzem.resetEnergy();
      }
    }
    getReadings();
    time2 = micros();
  }

}

void getReadings() {



  reading.voltage = pzem.voltage();
  reading.current = pzem.current();
  reading.power   = pzem.power();
  reading.energy  = pzem.energy();



  if (isnan(reading.voltage) || isnan(reading.current) || isnan(reading.energy)) {
    digitalWrite(pinLED, HIGH);
    delay(350);
    digitalWrite(pinLED, LOW);
    delay(350);
    return;
  }

  Serial.println("Voltage:  " + String(reading.voltage));
  Serial.println("Current:" + String(reading.current));

  JsonDocument doc;
  doc["voltage"] = reading.voltage;
  doc["current"] = reading.current;
  doc["power"] = reading.power;
  doc["energy"] = reading.energy;
  String jsonContent;
  serializeJson(doc, jsonContent);

  HTTPClient http;
  String url =
      "https://" +
      serverIP +
      "/insert";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  auto code = http.POST(jsonContent); 
  if (code != 200) {
    Serial.println("ERROR CODE: " + String(code));

    for(int i = 0; i < 3; i++){
      digitalWrite(pinLED, HIGH);
      delay(150);
      digitalWrite(pinLED, LOW);
      delay(150);
    }
    
  }
}
