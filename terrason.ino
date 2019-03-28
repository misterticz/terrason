/*
 * TERRASON is likely a solution to emulate the thermo control pro 2 with an ESP8266 based unit "SONOFF TH" (10 or 16)
 * 
 * Only power differentiate sonoff SONOFF and SONOFF TH16.
 * 
 */

// Todo : remove ntp from this file and put him in wifi file
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>

/* 
 * Application includes
 * NTP are so simple, no need to rewrite a class for that 
 */
#include "terrason.h"
#include "led.h"
#include "wifi.h"
#include "sensors.h"
#include "web.h"
#include "button.h"
#include "relay.h"

// NTP instanciation
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600*2, NTP_UPDATE_DELAY); // 2 hours of time offset, summer time for france

// Object of the application
static ApplicationConfiguration appconfig;

Button*     button;
Sensors*    sensors;
Wifi*       wifi;
Webserver*  web;
Led*        led;
Relay*      relay;

void buttonShortPush() {
  if (!wifi->assocMode()) {
    /* Tell IP on led */
    wifi->displayLastIPDigit();
  } else {
    /* Cancel wifi Assoc mode */
    wifi->stopAssocWifiPortal();
    wifi->startApplicationWifiPortal();
  }
}

void buttonLongPush() {
  /*
   * Pass into wifi configuration AP MODE
   */
  if (!wifi->assocMode()) {
    wifi->startAssocWifiPortal();
    web->startSetupWifiPortal();
  }
}

void cbAfterConnection() {
  web->startApplicationPortal();
}

void cbAfterSetupAssocPortal() {
  // Nothing to do
}


void setup() {

  // Start serial
  Serial.begin(115200);
  while (!Serial) { delay(10); } // wait for serial be ready
  Serial.print("Terrason v");
  Serial.print(version);
  Serial.println(" starting.");

//  byte* myptr = (byte *) &appconfig;
//  for (int i=0; i<sizeof(appconfig); i++) {
//    myptr[i] = EEPROM.read(i);
//  }
  EEPROM.begin(512);
  EEPROM.get(0,appconfig);
  //Serial.println(EEPROM.length());
  
  // Wifi
  wifi = new Wifi("terrason",
                  &cbAfterSetupAssocPortal,
                  &cbAfterConnection);
  
  wifi->startApplicationWifiPortal(); // configuration is stored in flash memory
                                      // Web service will be resumed in a callback

  // Led
  led = new Led(STH_LED);
  
  // Setup button
  button = new Button(STH_BUTTON, &buttonShortPush, &buttonLongPush);

  // Setup sensors
  sensors = new Sensors(STH_SENSOR);

  // Setup relay
  relay = new Relay(STH_RELAY);

  // Setup web
  web = new Webserver(sensors, relay, &timeClient);
}

void loop() {
  // Main loop, to run repeatedly:

  // Let's complain about the led
  if(!sensors->getMeasurement().tempvalid) {
    led->setMode(LED_HARD_BLINK);
  } else if(!wifi->clientConnected()) {
    led->setMode(LED_SOFT_BLINK);
  } else {
    led->setMode(LED_ON);
  }

  /* Update LED status */
  led->loop();

  /* Update Relay state */
  relay->loop();

  /* Check button state */
  button->loop();
  
  /*
   * Long-time background operation at the end, after LED management
   * tu get more fluidity
   */

  /* Update sensors values */
  sensors->loop();

  /* Reconnect wifi if needed */
  wifi->loop();

  /* Web services functions */
  web->loop();

}
