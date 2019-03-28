/*
 * Library for reading DHT / SONOFF2301/7021 Temp / Hum
 */

#ifndef _DHT_H_
  #define _DHT_H_

  #define DHT11   11
  #define DHT12   12
  #define DHT22   22
  #define AM2301  23

  #define SI7021  70
  #define SENSOR_NAME_SI7021 "SI7021 (SONOFF)"
  
  #define error_no_reading          1
  #define error_protocol_timeout    2
  #define error_checksum_error      3
  #define error_invalid_NAN_reading 4
  #define info_temperature          5
  #define info_humidity             6 

  class sensorsDHT {

    private:
      bool  _debug = true;
      byte  pin;
      bool  waitState(int state);
      int   read_dht_dat();
    
    public:    
      float temperature, humidity;
            sensorsDHT(byte pin);
      bool  read(byte sensortype);


    protected:
      template <typename Generic>
      void DEBUG(Generic text);    


  };

#endif
