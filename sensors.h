/*
 * Terrason sensors facilities
 * Actually only tested working with DS18B20
 */

#include <OneWire.h>
#include <DallasTemperature.h>
#include "dht.h"

#ifndef _SENSORS_H_:

  #define _SENSORS_H_

  #define MEASURE_INTERVAL 1000 // 1 second between each measure

  #define NO_SENSOR 0
  #define SONOFF_DS18B20 1
  #define SONOFF_SI7021 2
  
  struct Measurement {
    float temp;
    bool tempvalid;
    float hum;
    bool humvalid;
    unsigned long time;
    String sensorname;
    byte sensortype;
  };

  class Sensors {

    private:

      bool                  _debug = true;
      Measurement           measure = { 0.0, false, 0.0, false, 0};
      char                  gpio;
      OneWire*              oneWire;
      DallasTemperature*    dallas;
      sensorsDHT*           dht;
      template <typename Generic>
      void                  DEBUG(Generic text);

    public:
                            Sensors(byte gpio);
      void                  loop(); // Update measure
      Measurement           getMeasurement();

  };

#endif
