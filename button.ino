#include <Arduino.h>
#include "button.h"
    
Button::Button(int gpioButton, void (*cbShortPress)(), void (*cbLongPress)()) {
  /*
   * Initialize the mechanic
   */
  DEBUG("Initialize button ");
  this->gpioButton = gpioButton;
  this->cbShortPress = cbShortPress;
  this->cbLongPress = cbLongPress;
  lastStateChange = millis();
  lastButtonState = false;
  long_push_event = false;
}

Button::Button(int gpioButton) {
  Button(gpioButton, (callback) 0, (callback) 0);
}
 
void Button::loop() {
  /* 
   *  Check the button state each loop and call the good function if needed on release
   */
  pinMode(gpioButton, INPUT);
  bool newstate = (digitalRead(gpioButton) == LOW) ? true: false;
  int now = millis();

  // Detect state change for short 'click', on release
  if (newstate != lastButtonState) {
    // State change
    if (!newstate) {      
      // That's a realase, how many time ? call the callback
      if (now - lastStateChange < TIME_LONG_PRESS) {
        DEBUG("Click");
        if (this->cbShortPress) (cbShortPress)();
      }
    } else {
      long_push_event = false;
    }
    lastStateChange = now;
  }

 // Don't wait release for a long click, release when timeout
 if  (newstate \
  and now - lastStateChange >= TIME_LONG_PRESS \
  and !long_push_event) {
    DEBUG("Long push");
    long_push_event = true;
    if (this->cbLongPress) (cbLongPress)();
  }

  // Update object with newest values
  lastButtonState = newstate;
}

template <typename Generic>
void Button::DEBUG(Generic text) {
  if (_debug) {
    Serial.print("*BUTTON: ");
    Serial.println(text);
  }
}
