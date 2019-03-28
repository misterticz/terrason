#ifndef _RELAY_H_
  #define _RELAY_H_

  #include <NTPClient.h>
  #include "terrason.h"

  class Relay {

    private:

      bool                            _debug = true;
      byte                            gpio;
      bool                            state = false;
      void                            setRelay(bool state);

    public:
                                      Relay(byte gpio);
      bool                            isDay(unsigned int hour, unsigned int minute);
      bool                            currentState();
      void                            loop();

    protected:
      template <typename Generic>
      void  DEBUG(Generic text);
  };

#endif
