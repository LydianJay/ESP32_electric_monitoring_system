#include <Arduino.h>
#include <ArduinoJson.h>
#include <Wire.h> 
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define RXD2 16
#define TXD2 17

#define SSID "GayAssFaggot"
#define PASSWORD "Q4gp5h7o"

const String serverIP = "192.168.254.103";

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
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    delay(320);
  }
  Serial.println(WiFi.localIP());
}

void loop() {
  
  getReadings();
  delay(2000);
}

void getReadings() {



  reading.voltage = pzem.voltage();
  reading.current = pzem.current();
  reading.power = pzem.power();
  reading.energy = pzem.energy();
  reading.pf = pzem.pf();

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
      "http://" +
      serverIP +
      "/backend/index.php";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  auto code = http.POST(jsonContent); 
  if (code != 200) {
    Serial.println("ERROR CODE: " + String(code));
  }
}
