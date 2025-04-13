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



const String serverIP         = "eleksi.lyncxus.online";
const char *ntpServer         = "time.windows.com";
const long gmtOffset_sec      = 8 * 3600; // e.g. for Philippines (GMT+8)
const int daylightOffset_sec  = 0;
float energyOffset            = 11.262; // I have accidentally rest energy level so we need to add this.


struct PZEMReading {
  float voltage;
  float power;
  float current;
  float pf;
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
  if(!wm.autoConnect("eleksi", "admin123")){
    ESP.restart();
  }
  
  Serial.println("Connecting...");
  
  digitalWrite(pinLED, LOW);
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
}

void loop() {
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    int day = timeinfo.tm_mday;
    if (day == 1) {
      Serial.println("Today is the first day of the month! Reset ENERGY");
      energyOffset = 0;
      pzem.resetEnergy();
    }
  }
  getReadings();

  delay(55000); // every 55 seconds
}

void getReadings() {



  reading.voltage = pzem.voltage();
  reading.current = pzem.current();
  reading.power   = pzem.power();
  reading.energy  = pzem.energy() + energyOffset;
  reading.pf      = pzem.pf();



  if (isnan(reading.voltage) || isnan(reading.current) || isnan(reading.energy)) {
    return;
  }

  Serial.println("Voltage:  " + String(reading.voltage));
  Serial.println("Current:" + String(reading.current));

  JsonDocument doc;
  doc["voltage"] = reading.voltage;
  doc["current"] = reading.current;
  doc["power"] = reading.power;
  doc["energy"] = reading.energy;
  doc["power_factor"] = reading.pf;
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
    
    ESP.restart();
  }
}
