#include <ArduinoJson.h>

#include <Wire.h> 
#include <Arduino.h>
#include <PZEM004Tv30.h>
#include <WiFiManager.h>
#include <ESP32_Supabase.h>
#include "credentials.h"

#define RXD2 16
#define TXD2 17

struct PZEMReading {
  float voltage;
  float power;
  float current;
  float pf;
  float energy;
};


struct SendData {
  float voltage;
  float power;
  float energy;
};

PZEMReading reading;
PZEM004Tv30 pzem(Serial2, RXD2, TXD2);

Supabase db;
void getReadings() {
  reading.voltage = pzem.voltage();
  reading.current = pzem.current();
  reading.power = pzem.power();
  reading.energy = pzem.energy();
  reading.pf = pzem.pf();
  
}



void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  //WiFiManager wm;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay(120);
  } 
  Serial.println("Connected To WiFi...");
  

  //wm.autoConnect(WIFI_SSID, WIFI_PASSWORD);
  db.begin(SUPABASE_URL, SUPABASE_ANON_KEY);
  Serial.println("Login Result: " + String(db.login_email(EMAIL, PASS)));
  getReadings();
  JsonDocument doc;
  doc["voltage"] = reading.voltage;
  doc["current"] = reading.current;
  doc["power"] = reading.power;
  doc["energy"] = reading.energy;
  doc["power_factor"] = reading.pf;
  String jsonContent;
  serializeJson(doc, jsonContent);
  db.urlQuery_reset();
  Serial.println("Code: " + String(db.insert("Reading", jsonContent, false)));


  delay(60 * 1000);
}

void loop() {
    
  getReadings();

  Serial.println("Voltage: " + String(reading.voltage));
  Serial.println("Current: " + String(reading.current));
  Serial.println("Power: " + String(reading.power));
  Serial.println("Energy: " + String(reading.energy));
  Serial.println("Power Factor: " + String(reading.pf));


  JsonDocument doc;
  doc["voltage"] = reading.voltage;
  doc["current"] = reading.current;
  doc["power"] = reading.power;
  doc["energy"] = reading.energy;
  doc["power_factor"] = reading.pf;
  String jsonContent;
  serializeJson(doc, jsonContent);
  db.urlQuery_reset();
  Serial.println("Code: " + String(db.insert("Reading", jsonContent, false)));

  delay(60 * 1000);
  
}

