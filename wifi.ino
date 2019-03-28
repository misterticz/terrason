/*
 * Wifi utilities
 */
#include "wifi.h"

Wifi::Wifi(char* apname_base, void (*cbAfterSetupAssocPortal)(), void (*cbAfterConnection)()) {
  /*
   * Construct object wifi
   */
  this->cbAfterSetupAssocPortal = cbAfterSetupAssocPortal;
  this->cbAfterConnection = cbAfterConnection;
  AP_SSID = String(apname_base) + String(ESP.getChipId());
}

void Wifi::startAssocWifiPortal() {
  /*
   * Create captive portal
   */

  DEBUG(F("Initialize AP mode for WIFI configuration by user"));
  
  this->config_portal_start_on = millis();
  
  // Stop any client connection then pass into Access Point Station
  //WiFi.persistent(false); // not sure
  //WiFi.disconnect(); // not sure
  
  WiFi.setAutoConnect(false);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID.c_str());
  //WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  delay(500);
  DEBUG(F("AP IP address: "));
  DEBUG(WiFi.softAPIP());
  
  //dnsServer->stop(); // Needed ?
  dnsServer.reset(new DNSServer());

  /* Setup the DNS server redirecting all the domains to the apIP */
  dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer->start(DNS_PORT, "*", WiFi.softAPIP());

}

bool Wifi::assocMode() {
  return config_portal_start_on != 0;
}

void Wifi::stopAssocWifiPortal() {
  DEBUG(F("Stop AP mode"));
  dnsServer.reset();
  webServer.reset();
  WiFi.softAPdisconnect();
  WiFi.mode(WIFI_OFF);
  config_portal_start_on = 0;
}

void Wifi::startApplicationWifiPortal() {
  // Connect with known parameters
  connect(config);
}

void Wifi::connect(wificonfiguration config) {

  DEBUG(F("Connecting as wifi client to AP"));
  connection_in_progress = true;

  //webServer.reset();

  // check if we've got static_ip settings, if we do, use those.
  //if (_sta_static_ip) {
  //  this->DEBUG(F("Custom STA IP/GW/Subnet"));
  //  WiFi.config(_sta_static_ip, _sta_static_gw, _sta_static_sn);
  //  this->DEBUG_WM(WiFi.localIP());
  //}
  //fix for auto connect racing issue
  //if (WiFi.status() == WL_CONNECTED) {
  //  this->DEBUG(F("Already connected. Bailing out."));
  //}
  //check if we have ssid and pass and force those, if not, try with last saved values
  //WiFi.begin(config.ssid, config.password);

  WiFi.setAutoConnect(true);
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin();
  
//  } else {
//    if (WiFi.SSID()) {
//      this->DEBUG(F("Using last saved values, should be faster"));
//      //trying to fix connection in progress hanging
//      ETS_UART_INTR_DISABLE();
//      wifi_station_disconnect();
//      ETS_UART_INTR_ENABLE();

//      WiFi.begin();
//    } else {
//      this->DEBUG(F("No saved credentials"));
//    }
//  }

  //not connected, WPS enabled, no pass - first attempt
//  #ifdef NO_EXTRA_4K_HEAP
//  if (_tryWPS && connRes != WL_CONNECTED && pass == "") {
//    startWPS();
    //should be connected at the end of WPS
//    connRes = waitForConnectResult();
//  }
//  #endif

}

void Wifi::loop() {
  /*
   * Client connection will retry permanently to reconnect to the AP station
   * Check every WIFI_RETRY_DELAY seconds wifi connection end reconnect if
   * not connected 
   */
  uint8_t status = WiFi.status();
  if (connection_in_progress) {
    if (status == WL_CONNECTED || status == WL_CONNECT_FAILED) {
        connection_in_progress = false;
        if (status == WL_CONNECTED) {
          WiFi.mode(WIFI_STA);
          this->DEBUG(F("Connected."));
          timeClient.begin();
          if (timeClient.forceUpdate()) {
            DEBUG(F("NTP time updated successfully."));
          } else {
            DEBUG(F("NTP time client forceUpdate failed"));
          }
          // Call the callback function if needed
          if (this->cbAfterConnection) (*this->cbAfterConnection)(); 
        } else {
          this->DEBUG(F("WIFI Connection failed"));          
        }
    }
  }

  if (config_portal_start_on != 0) {
    if (millis() > config_portal_start_on + WIFI_ASSOC_PORTAL_TIMEOUT) {
      /* Assoc portal timeout, reconnect application portal */
      stopAssocWifiPortal();
      startApplicationWifiPortal();
    } else {
      /* Process request for captive assoc portal */
      dnsServer->processNextRequest();
    }
  }

  /* If wifi connected in client mode, update the time */  
  if (wifi->clientConnected()) { timeClient.update(); } 

}

void Wifi::displayLastIPDigit() {
  displayLastIPDigit(WiFi.localIP()[3]);
}

void Wifi::displayLastIPDigit(int lastipnumber) {
  /* 
   *  Tell on led the ip adress on SONOFF LED (STH_LED_GPIO12)
   */
  int factor = 100;
  byte digit;

  DEBUG(F("Starting telling IP client address on LED"));
  led->setMode(LED_OFF);
  delay(WIFI_TELLIP_TIMEBETWEENDIGIT);

  while (factor != 0) {
    digit = (lastipnumber / factor) % 10;
    if (!digit) {
      // Start a zero long notification
      led->setMode(LED_ON);
      delay(WIFI_TELLIP_TIMEZERO);
      led->setMode(LED_OFF);
    } else {
      // Start somes short notification
      for (byte i=0; i<digit; i++) {
        led->setMode(LED_ON);
        delay(WIFI_TELLIP_TIMEDIGIT);
        led->setMode(LED_OFF);
        delay(WIFI_TELLIP_TIMEDIGIT);  
        }
    }
    factor = factor / 10;
    if (factor > 0) {
      delay(WIFI_TELLIP_TIMEBETWEENDIGIT);
    }
  }
  delay(WIFI_TELLIP_TIMEBETWEENDIGIT);

}

bool Wifi::clientConnected() {
  return  WiFi.status() == WL_CONNECTED
    &&    WiFi.getMode() == WIFI_STA;
}

template <typename Generic>
void Wifi::DEBUG(Generic text) {
  if (_debug) {
    Serial.print("*WIFI: ");
    Serial.println(text);
  }
}
