#ifndef _TERRASON_H_
  #define _TERRASON_H_

  #define _TERRASON_DEBUG_

  /* PROGRAM SPECIFICS */
  #define version "0.1dev"
  #define _TERRASON_TITLE_ "Terrason"

  /* SONOFF TH10/16 SPECIFICS */
  #define STH_BUTTON 0
  #define STH_LED 13
  #define STH_RELAY 12
  #define STH_SENSOR 14

  /* NTP related */
  #define NTP_UPDATE_DELAY 1000*60*15 // Update time all 15 minutes

  /* STRUCTURES */
  struct ApplicationConfiguration {

    // Functions configuration
    unsigned short int daystarthour;
    unsigned short int daystartminute;
    unsigned short int daystophour;
    unsigned short int daystopminute;
    unsigned short int daytemp;
    unsigned short int nighttemp;
    unsigned short int heatlessthan;

  };

#endif
