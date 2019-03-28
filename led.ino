/*
 * Code for the class Led
 */
 
 #include "led.h"

Led::Led(byte gpio) {
  this->DEBUG("Initialize LED");
  this->gpio = gpio;
  pinMode(gpio, OUTPUT);
  this->setMode(LED_ON);
}

void Led::setLed(byte value) {
  /* 
   * Value are LOW or HIGH 
   * but on SONOFF TH10 REV2.1 it's lighting when it's low and dark true ?
   * 
   * THE CODE INVERT THE PARAM VALUE !
   */

  // Protect for multiple calls
  if (ledvalue == value) return;
  digitalWrite(this->gpio, (value==HIGH) ? LOW : HIGH);
  ledvalue = value;
}

void Led::setMode(byte mode) {
  // Don't broke the timing
  
  // The function don't change
  // working blinking mode
  if (mode == this->mode) return;

  start_time = millis();
  this->mode = mode;
  switch (this->mode = mode) {
    case LED_OFF:
      DEBUG("set mode off");
      setLed(LOW);
      break;
    case LED_ON:
      setLed(HIGH);
      DEBUG("set mode on");
      break;
    case LED_SOFT_BLINK:
      DEBUG("set mode soft blinking");
      setLed(HIGH);
      break;
    case LED_HARD_BLINK:
      DEBUG("set mode hard blinking");
      setLed(HIGH);
      break;
  } 
}

void Led::loop() {

  // No need to change
  if (mode != LED_SOFT_BLINK && mode != LED_HARD_BLINK) { return; }
    
  unsigned long now= millis(), timeon, timetotal;
  switch (mode) {
    case LED_SOFT_BLINK:
      timeon = SOFT_TIME_ON;
      timetotal = SOFT_TIME_TOTAL;
      break;
    case LED_HARD_BLINK:
      timeon = HARD_TIME_ON;
      timetotal = HARD_TIME_TOTAL;
      break;
  }

  if (now - start_time > timetotal) {
    this->setLed(HIGH);
    // Do it with a delay if small value
    // That's bloking !!
    start_time = millis();
    if (timeon < 200) {
      delay(timeon);
      this->setLed(LOW);
    }
  } else if ( (now - start_time) % timetotal > timeon ) {
    this->setLed(LOW);
  } 
}

template <typename Generic>
void Led::DEBUG(Generic text) {
  if (_debug) {
    Serial.print("*LED: ");
    Serial.println(text);
  }
}
