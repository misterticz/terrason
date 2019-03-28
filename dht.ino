/*
 * Code for DHT serie sensor specially for SONOFF2301/7021
 * 
 * heavely inspired by ESPEasy DHT sensors lib
 * https://github.com/letscontrolit/ESPEasy/blob/3e20f617eafa0ed4bfa064943189f6e2623e234d/src/_P005_DHT.ino
 * 
 */

#include "dht.h"

sensorsDHT::sensorsDHT(byte pin) {
  DEBUG("Initialize DHT11/12/22 - SONOFF 2301/7021");
  this->pin = pin;
}

bool sensorsDHT::waitState(int state) {
  unsigned long timeout = micros() + 100;
  while (digitalRead(pin) != state)
  {
    if (micros() > timeout) return false;
    delayMicroseconds(1);
  }
  return true;
}

bool sensorsDHT::read(byte sensortype) {
  byte i;

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);              // Pull low
  switch (sensortype) {
    case DHT11: case DHT22: case DHT12: delay(18); break;  // FIXME TD-er: Must this be so long?
    case AM2301: delayMicroseconds(900); break;
    case SI7021: delayMicroseconds(500); break;
  }
  switch (sensortype) {
    case DHT11: case DHT22: case DHT12: case AM2301:
      pinMode(pin, INPUT);
      delayMicroseconds(50);
      break;
    case SI7021:
      // See: https://github.com/letscontrolit/ESPEasy/issues/1798
      digitalWrite(pin, HIGH);
      delayMicroseconds(20);
      pinMode(pin, INPUT);
      break;
  }
  if(!waitState(0)) { return false; }
  if(!waitState(1)) { return false; }
  noInterrupts();
  if(!waitState(0)) { return false; }
  bool readingAborted = false;
  byte dht_dat[5];
  for (i = 0; i < 5 && !readingAborted; i++)
  {
      byte data = read_dht_dat();
      if(data == -1)
      {   DEBUG("Timeout from sensor");
          readingAborted = true;
      }
      dht_dat[i] = data;
  }
  interrupts();
  if (readingAborted)
    return false;

  // Checksum calculation is a Rollover Checksum by design!
  byte dht_check_sum = (dht_dat[0] + dht_dat[1] + dht_dat[2] + dht_dat[3]) & 0xFF; // check check_sum
  if (dht_dat[4] != dht_check_sum)
  {
      DEBUG("Checksum error");
      return false;
  }

  float temperature = NAN;
  float humidity = NAN;
  switch (sensortype) {

    case DHT11:
      temperature = float(dht_dat[2]); // Temperature
      humidity = float(dht_dat[0]); // Humidity
      break;

    case DHT12:
      temperature = float(dht_dat[2]*10 + (dht_dat[3] & 0x7f)) / 10.0; // Temperature
      if (dht_dat[3] & 0x80) { temperature = -temperature; } // Negative temperature
      humidity = float(dht_dat[0]*10+dht_dat[1]) / 10.0; // Humidity
      break;

    case DHT22: case AM2301: case SI7021:
      if (dht_dat[2] & 0x80) // negative temperature
        temperature = -0.1 * word(dht_dat[2] & 0x7F, dht_dat[3]);
      else
        temperature = 0.1 * word(dht_dat[2], dht_dat[3]);
      humidity = 0.1 * word(dht_dat[0], dht_dat[1]); // Humidity
      break;
  }

  if (temperature == NAN || humidity == NAN)
  {     //P005_log(event, P005_error_invalid_NAN_reading);
        DEBUG("Temperature or humidity read error");
        return false;
  }

  this->temperature = temperature;
  this->humidity = humidity;
  return true;
}

int sensorsDHT::read_dht_dat() {
  byte i = 0;
  byte result = 0;
  for (i = 0; i < 8; i++)
  {
    if (!waitState(1))  return -1;
    delayMicroseconds(35); // was 30
    if (digitalRead(pin))
      result |= (1 << (7 - i));
    if (!waitState(0)) return -1;
  }
  return result;
}

template <typename Generic>
void sensorsDHT::DEBUG(Generic text) {
  if (_debug) {
    Serial.print("*SENSOR DHT: ");
    Serial.println(text);
  }
}
