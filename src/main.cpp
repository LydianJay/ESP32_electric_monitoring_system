#include <Wire.h> 
#include <Arduino.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>

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
}

void loop() {
    
    delay(1000);
    getReadings();


    Serial.println("Voltage: " + String(reading.voltage));
    Serial.println("Current: " + String(reading.current));
    Serial.println("Power: " + String(reading.power));
    Serial.println("Energy: " + String(reading.energy));
    Serial.println("Power Factor: " + String(reading.pf));


    delay(1000);
  
}

