/*
 * Web service for terrason
 */
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include "terrason.h"
#include "sensors.h"
#include "relay.h"

#ifndef _WEB_H_
  #define _WEB_H_
  
  /*
   * HTML TEMPLATING
   */
  const char HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
  const char HTTP_STYLE[] PROGMEM           = "<style>\
.c{text-align: center;}\
body{text-align: center;font-family:verdana;}\
.q{float: right;width: 64px;text-align: right;Ò}\
.l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}\
</style>";
  const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
  const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div id='menu'>\
<b><a href='/' alt='terrason'>Terrason</a></b> | \
<b><a href='/settings' alt='Settings'>Settings</a></b> | \
<b><a href='/wifi' alt='Configuration wifi'>Wifi</a></b> | \
<b><a href='/deviceinfo' alt='Device informations'>Device</a></b> | \
<b><a href='/resetdevice' alt='Reset the device' />Reset</a></b> \
</div>";
  const char HTTP_END[] PROGMEM             = "</body></html>";

  // Capture portal for wifi configuration specifics
  const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
  const char HTTP_WIFI_ITEM[] PROGMEM       = "<tr><td><a href='#p' onclick='c(this)'>{v}</a></td><td><span class='q {i}'>{r}%</span></td></tr>";
  const char HTTP_WIFI_FORM[] PROGMEM       = "<tr><td>SSID / Wifi network</td><td><input id='s' name='s' length=32 placeholder='SSID'></td></tr>\
  <tr><td>Password</td><td><input id='p' name='p' length=64 type='password' placeholder='password'></td></tr>";
//  const char HTTP_FORM_PARAM[] PROGMEM      = "<br/><input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
  const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>Save</button></form>";
  const char HTTP_SCAN_LINK[] PROGMEM       = "<div class=\"c\"><button name='Scan' type='button'>Scan</button></div>";
  const char HTTP_SAVED[] PROGMEM           = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";

  // Application portal specifics
const char HTTP_APP_STYLE[] = "<style>\
body{padding:0; margin:0; border:0;};\
h1 { font-family: Arial, 'Helvetica Neue', Helvetica, sans-serif; font-size: 24px; font-style: normal; font-variant: normal; font-weight: 700; line-height: 26.4px; } h3 { font-family: Arial, 'Helvetica Neue', Helvetica, sans-serif; font-size: 14px; font-style: normal; font-variant: normal; font-weight: 700; line-height: 15.4px; } p { font-family: Arial, 'Helvetica Neue', Helvetica, sans-serif; font-size: 14px; font-style: normal; font-variant: normal; font-weight: 400; line-height: 20px; } blockquote { font-family: Arial, 'Helvetica Neue', Helvetica, sans-serif; font-size: 21px; font-style: normal; font-variant: normal; font-weight: 400; line-height: 30px; } pre { font-family: Arial, 'Helvetica Neue', Helvetica, sans-serif; font-size: 13px; font-style: normal; font-variant: normal; font-weight: 400; line-height: 18.5714px; }\
#menu { border-bottom: 3px; solid #bbbbbb; background: #eeeeee; padding:10px; width:100%;}\
#content { padding: 0.3em; }\
a:link, #menu a:visited { color: #555555;}\
</style>";

const char HTTP_APP_DEVICE_INFO[] = "<div id='content'>\
  <table>\
    <tr><td>Chip ID</td><td><b>%chipid%</b></td></tr>\
    <tr><td>Flash Chip ID</td><td><b>%flashchipid%</b></td></tr>\
    <tr><td>IDE Flash size</td><td><b>%ideflashsize%</b></td></tr>\
    <tr><td>Real flash size</td><td><b>%realflashsize%</b></td></tr>\
    <tr><td>Soft AP IP</td><td><b>%softapip%</b></td></tr>\
    <tr><td>Soft AP MAC</td><td><b>%softapmac%</b></td></tr>\
    <tr><td>Station MAC</td><td><b>%stationmac%</b></td></tr>\
  </table>\
</div>";

const char HTTP_APP_ROOT[] = "<div id='content'>\
  <table>\
    <tr><td>Date</td><td>%hour%</td></tr>\
    <tr><td>Temperature</td><td>%temp%</td></tr>\
    <tr><td>Humidity</td><td>%hum%</td></tr>\
    <tr><td>Sensor</td><td>%sensorname%</td></tr>\
    <tr><td>Output</td><td>%relaystate% %isday%</td></tr>\
  </table>\
</div>";

const char HTTP_APP_SETTINGS[] = "\
<form method='post'>\
  <table>\
    <tr>\
      <td>Day start</td><td><input type='text' name='daystarthour' value='%daystarthour%' size='2' maxlength='2' placeholder='%daystarthour%' /> : <input type='text' name='daystartminute' value='%daystartminute%' size='2' maxlength='2' placeholder='%daystartminute%' /> (24h format)</td>\
    </tr>\
    <tr>\
      <td>Day end</td><td><input type='text' name='daystophour' value='%daystophour%' size='2' maxlength='2' placeholder='%daystophour%' /> : <input type='text' name='daystopminute' value='%daystopminute%' size='2' maxlength='2' placeholder='%daystopminute%' /> (24h format)</td>\
    </tr>\
    <tr>\
      <td>Day Temp</td><td><input type='text' name='daytemp' value='%daytemp%' size='2' maxlength='2' placeholder='%daytemp%' /> °C</td>\
    </tr>\
    <tr>\
      <td>Night Temp</td><td><input type='text' name='nighttemp' value='%nighttemp%' size='2' maxlength='2' placeholder='%nighttemp%' /> °C</td>\
    </tr>\
    <tr>\
      <td>Start heat when T° </td><td><input type='text' name='heatlessthan' size='2' maxlength='2' value='%heatlessthan%' placeholder='%heatlessthan%' /> °C lesser than</td>\
    </tr>\
    <tr>\
      <td colspan='2'>%message%<input type='submit' value='Save' name='submit' /> <input type='reset' value='Reset' name='submit' /></td>\
    </tr>\
  </ul>\
</form>";

  const char HTTP_APP_RESET[] = "<div id='content'><form method='post'>Are you sure you want to reset the device ?<br /><button type='submit' name='performreset' value='performreset'>Perform reset</button</form></div>\
                                 <div id='content'><form method='post'>Activate developper OTA (for arduino IDE)<br /><button type='submit' name='startota' value='start'>Start OTA</button</form></div>";
  const char HTTP_APP_RESET_DONE[] = "<div id='content'>Reset sent to device. Please reconnect to device after few seconds.</div>";
  
  class Webserver {
    /*
     * Handle WEBSERVER utilities
     */
    private:
      std::unique_ptr<ESP8266WebServer> webServer;
      NTPClient*                        timeClient;
      Sensors*                          sensors;
      Relay*                            relay;
      bool                              _debug = true;

      //unsigned short int daystart = 8;
      //PROGMEM ApplicationConfiguration appconf;

      int           _paramsCount            = 0;
      int           _minimumQuality         = -1;
      boolean       _removeDuplicateAPs     = true;
      boolean       _shouldBreakAfterConfig = false;
      boolean       _tryWPS                 = false;

      String        _ssid                   = "";
      String        _pass                   = "";

    public:
                Webserver( Sensors* sensors, Relay* relay, NTPClient* timeClient );
      void      startApplicationPortal();
      void      startSetupWifiPortal();
      void      loop();

    protected:
      /* 
       * Helpers 
       */
      template <typename Generic>
      void      DEBUG(Generic text);
      int       getRSSIasQuality(int RSSI);
      boolean   isIp(String str);
      String    toStringIp(IPAddress ip);
      bool      isInteger(String str);

      /*
       * Application portal
       */
      void      handleApplicationRoot();
      void      handleApplicationSettings();
      void      handleApplicationTemp();
      void      handleApplicationHum();
      void      handleApplicationReset();
      
      /*
       * Captive wifi / web portal handlers web oages
       */
      void          handleAPRoot();
      void          handleAPWifi(boolean scan);
      void          handleAPWifiSave();
      void          handleAPInfo();
      void          handleAPReset();
      void          handleAPNotFound();
      void          handle204();
      boolean       captivePortal();
};

#endif
