/*
 * Wifi utilities
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <string.h>

#ifndef _WIFI_H_
  #define _WIFI_H_

  #define DNS_PORT 53
  #define WIFI_ASSOC_SSID "terrason"
  #define WIFI_ASSOC_PORTAL_TIMEOUT 5000 * 60 // 5 secondes * 60 = 5 minutes
  
  #define WIFI_TELLIP_TIMEZERO 1000 // Time of a zero
  #define WIFI_TELLIP_TIMEDIGIT 300 // Time for describe a digit
  #define WIFI_TELLIP_TIMEBETWEENDIGIT 1000 // Time between 2 digits
  
  struct wificonfiguration {
    char ssid[64];
    char password[64];
  };
  
  class Wifi {

    /*
     * This library handle wifi connection
     * - WIFI setup AP
     * - WIFI client connection, from settings
     * - Callback on configuration, and on client connection
     * Connection are asynchronous and no bloking
     */

    private:

      std::unique_ptr<DNSServer>          dnsServer;
      std::unique_ptr<ESP8266WebServer>   webServer;
      bool                                _debug = true;
      bool                                connection_in_progress = false;
      unsigned long                       config_portal_start_on = 0;     // time of last start configuration access point
      void                                (*cbAfterConnection)();         // callback after setup and connection successfull
      void                                (*cbAfterSetupAssocPortal)();   // callback after successfull connection (called after AfterSetup in configuration)
      char*                               apname_base;                    // Prefix for AP name (will be combined with chip id
      String                              AP_SSID;
      wificonfiguration                   config;                         // Current configuration used

    public:

                            Wifi(char* apname_base, void (*cbAfterSetup)(), void (*cbAfterConnection)());
      void                  startAssocWifiPortal();
      void                  stopAssocWifiPortal();
      void                  startApplicationWifiPortal();
      void                  connect(wificonfiguration config);
      void                  loop();
      bool                  clientConnected();
      bool                  assocMode();
      void                  displayLastIPDigit();
      void                  displayLastIPDigit(int lastipnumber);

    protected:
    
      template <typename Generic>
      void                  DEBUG(Generic text);

  };

#endif
