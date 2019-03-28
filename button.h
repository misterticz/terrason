/*
 * This library permit the system to have two callbacks function on:
 *  - shortButtonPush
 *  - longButtonPush
 */

#ifndef _BUTTON_H_
  #define _BUTTON_H_
  #define TIME_LONG_PRESS 5000

  typedef void (*callback)();

  class Button {

    private:
      bool  _debug = true;
      int   gpioButton;
      bool  lastButtonState = false;
      int   lastStateChange;
      bool  long_push_event = false;
      void  (*cbShortPress)();
      void  (*cbLongPress)();

    public:
            Button(int gpioButton);
            Button(int gpioButton, void (*cbShortPress)(), void (*cbLongPress)());
      void  loop();
      template <typename Generic>
      void  DEBUG(Generic text);
  };

#endif
