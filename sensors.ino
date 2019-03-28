/*
 * Sensors facilities
 */

#include "sensors.h"

Sensors::Sensors(byte gpio) {
  this->DEBUG(F("Initialize sensors"));
  oneWire = new OneWire(gpio);
    
  // Initialize DS18B20
  this->gpio = gpio;
  dallas = new DallasTemperature(oneWire);  
  dallas->begin();
  if (dallas->getDS18Count() == 1) {
    this->DEBUG("DS18B20 temperature detected.");
  }

  // Initialize SONOFF SI7021
  dht = new sensorsDHT(gpio);
  oneWire->reset();
  delayMicroseconds(5000);
  if (dht->read(SI7021)) {
    this->DEBUG(F("SI7021 temp/hum detected."));
  }

  this->DEBUG(F("* END SENSORS LIST!"));
}

void Sensors::loop() {
  // Don't do a measure now ?
  if (millis() < this->measure.time + MEASURE_INTERVAL) return;

  // Actuallty humidity sensors are not implemented
  measure.sensorname = "NOSENSOR";
  measure.sensortype = NO_SENSOR;
  measure.tempvalid = false;
  measure.temp = 0.0;
  measure.hum = 0.0;
  measure.humvalid = false;

  /* 
   * First try to read on same sensors 
   */
  if (measure.sensortype == SONOFF_DS18B20) {
      //dallas->requestTemperatures();
      measure.sensorname = F("DS18B20");
      dallas->requestTemperaturesByIndex(0);
      float temp = dallas->getTempCByIndex(0);
      //float temp = dallas->getTempC();
      if (temp != -127.0) {
        measure.temp = temp;
        measure.tempvalid = true;
        return;
      }
  } else if (measure.sensortype == SONOFF_SI7021) {
    if (dht->read(SI7021)) {
      measure.sensorname = F("SI7021");
      measure.tempvalid = true;
      measure.humvalid = true;
      measure.temp = dht->temperature;
      measure.hum = dht->humidity;
      return;
    }    
  }
  
  // If reading last known sensor doesn't work, so reset the bus and detect sensor
  oneWire->reset();
  delayMicroseconds(5000);

  if (!measure.tempvalid) {
    dallas->begin();
    if (dallas->getDS18Count() == 1) {
      DeviceAddress sensorDeviceAddress;
      dallas->getAddress(sensorDeviceAddress, 0); //Demande l'adresse du capteur à l'index 0 du bus
      dallas->setResolution(sensorDeviceAddress, 12); //Résolutions possibles: 9,10,11,12
      delayMicroseconds(20); // By security, often bad read of DS18B20
      dallas->requestTemperaturesByIndex();
      float temp = dallas->getTempCByIndex(0);
      //float temp = dallas->getTempC(sensorDeviceAddress);
      if (temp != -127.0) {
        measure.sensorname = F("DS18B20");
        measure.sensortype = SONOFF_DS18B20;
        measure.temp = temp;
        measure.tempvalid = true;
      }
    } 
  }
    
  if (!measure.tempvalid) {
    oneWire->reset();
    delayMicroseconds(5000);
    
    if (dht->read(SI7021)) {
      measure.sensorname = F("SI7021");
      measure.sensortype = SONOFF_SI7021;
      measure.tempvalid = true;
      measure.humvalid = true;
      measure.temp = dht->temperature;
      measure.hum = dht->humidity;
    }
  }


//  if (this->si7021->begin()) {
//    sensor_name = "SI7021";
//    this->measure.temp = si7021->readTemperature();
//    this->measure.tempvalid = true;
//    this->measure.hum = si7021->readHumidity();
//    this->measure.humvalid = true;
//  }
  measure.time = millis();
  
  char temp[8]; // sign, 3 digit, separator, and 2 digit so totaly 7 char + '0' ending
  char hum[5]; // sign, 3 digit, and '0' ending
  String debugline = String(measure.sensorname) + " ";
  if (this->measure.tempvalid) {
    dtostrf(this->measure.temp, 3, 2, temp);
    debugline += String(temp) + "°C";
  } else {
    debugline += "temperature is unavailable";
  }
  if (this->measure.humvalid) {
    dtostrf(this->measure.hum, 3, 2, hum);
    debugline += String(", ") + String(hum) + "%";
  } else {
    debugline += "";
  }
  this->DEBUG(debugline);  
}

Measurement Sensors::getMeasurement() {
  return this->measure;
}

template <typename Generic>
void Sensors::DEBUG(Generic text) {
  if (_debug) {
    Serial.print("*SENSORS: ");
    Serial.println(text);
  }
}
