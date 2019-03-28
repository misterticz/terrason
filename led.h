/*
 * Manage the led
 */

#ifndef _LED_H_
  #define _LED_H_

  #define LED_OFF 0
  #define LED_ON 1
  #define LED_SOFT_BLINK 2
  #define LED_HARD_BLINK 3

  // BLINK TIMINGS
  #define SOFT_TIME_ON 60
  #define SOFT_TIME_TOTAL 4000
  #define HARD_TIME_ON 250
  #define HARD_TIME_TOTAL 500
  

//  const unsigned long LED_SOFT_BLINK_PARAMS[2] = (50,4000);   // 100ms per second
//  const unsigned long [2] LED_HARD_BLINK_PARAMS = [100,4000];  // 500ms per second

  class Led {

    private:

      bool          _debug = true;
      byte          gpio;
      byte          mode = LED_OFF;
      unsigned long start_time;
      byte          ledvalue;
      template <typename Generic>
      void          DEBUG(Generic text);

    public:

                    Led(byte gpio);
      void          setLed(byte value);
      void          setMode(byte mode);
      void          loop();

  };

#endif
