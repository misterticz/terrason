#include "relay.h";

Relay::Relay(byte gpio) {
  DEBUG(F("Initialize RELAY"));
  this->gpio = gpio;
  pinMode(gpio, INPUT);
  state = digitalRead(gpio) == HIGH;
  if (state) DEBUG(F("Relay is ON")); else DEBUG(F("Relay is OFF"));
  pinMode(gpio, OUTPUT);
}

void Relay::setRelay(bool value) {
  /* 
   * Value are LOW or HIGH 
   * but on SONOFF TH10 REV2.1 it's lighting when it's low and dark true ?
   * 
   * THE CODE INVERT THE PARAM VALUE !
   */
  // Protect for multiple calls
  if (this->state == value) return;
  if (value) DEBUG(F("Set to ON (HEATING)")); else DEBUG(F("Set to OFF"));
  digitalWrite(this->gpio, (value) ? HIGH : LOW);
  this->state = value;
}

void Relay::loop() {
  // Determine if the relay must be ON or OFF
  int temporder;
  if ( isDay(timeClient.getHours(), timeClient.getMinutes())) {
    temporder = appconfig.daytemp;
  } else {
    temporder = appconfig.nighttemp;
  }
  
  // Get actual temp from sensors
  if (sensors->getMeasurement().temp < temporder - appconfig.heatlessthan) {
    setRelay(true);
  } else {
    setRelay(false);
  }
}

bool Relay::isDay(unsigned int hour, unsigned int minute) {
  // Retrieve hour and minute
  return (hour > appconfig.daystarthour || hour == appconfig.daystarthour && minute >= appconfig.daystartminute ) && \
         (hour < appconfig.daystophour || hour == appconfig.daystophour && minute <= appconfig.daystopminute );
}

bool Relay::currentState() {
  return state;
}

template <typename Generic>
void Relay::DEBUG(Generic text) {
  if (_debug) {
    Serial.print("*RELAY: ");
    Serial.println(text);
  }
}
