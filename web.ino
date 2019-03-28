/*
 * Web service for terrason
 */

#include <ESP.h>
#include <time.h>
#include "web.h"

/* INSTANCIATE THE WEB SERVICE */

Webserver::Webserver(Sensors* sensors, Relay* relay, NTPClient* timeClient) {
  // Instantiate webServer
  DEBUG(F("Initialize web serveur"));
  webServer.reset(new ESP8266WebServer(80));
  this->sensors = sensors;
  this->relay = relay;
  this->timeClient = timeClient;
}

void Webserver::startApplicationPortal() {
  /*
   * Homepage
   */
  this->DEBUG(F("Configure HTTP application portal"));  
  webServer->on("/", std::bind(&Webserver::handleApplicationRoot, this));
  webServer->on("/settings", std::bind(&Webserver::handleApplicationSettings, this));
  webServer->on("/wifi", std::bind(&Webserver::handleAPWifi, this, true));
  webServer->on("/wifisave", std::bind(&Webserver::handleAPWifiSave, this));
  webServer->on("/deviceinfo", std::bind(&Webserver::handleAPInfo, this));
  webServer->on("/temp", std::bind(&Webserver::handleApplicationTemp, this));
  webServer->on("/hum", std::bind(&Webserver::handleApplicationHum, this));
  webServer->on("/resetdevice", std::bind(&Webserver::handleApplicationReset, this)),
  
  // Start SERVER
  webServer->begin();
}

void Webserver::handleApplicationRoot() {
  DEBUG("Root (homepage)");
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", _TERRASON_TITLE_);
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_APP_STYLE);
//  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += FPSTR(HTTP_APP_ROOT);

  String temp = "<b style='color:red;'>/!\\ No sensor found /!\\</b>";
  if (sensors->getMeasurement().tempvalid) { temp = "<b>" + String(sensors->getMeasurement().temp) + "°C</b>"; }  
  String hum = "<b style='color:red;'>/!\\ No sensor found /!\\</b>";
  if (sensors->getMeasurement().humvalid) { hum = "<b>" + String(sensors->getMeasurement().hum) + "%</b>"; }  

  page.replace("%temp%", temp);
  page.replace("%hum%", hum);
  page.replace("%sensorname%", "<b>" + sensors->getMeasurement().sensorname + "</b>");
  
  time_t timenow = timeClient->getEpochTime();
  tm* timesplit = localtime(&timenow);
  String day, month, year, hour, minute;
  char buf[3];
  sprintf(buf,"%02d", timesplit->tm_mday);
  day = String(buf);
  sprintf(buf,"%02d", timesplit->tm_mon-1);
  month = String(buf);
  sprintf(buf,"%02d", timesplit->tm_year+1900);
  year = String(buf);
  page.replace(F("%hour%"), String(F("<b>")) + day + "/" + month + "/" + year + " " + timesplit->tm_hour + ":" + timesplit->tm_min + F("</b>"));
  if (relay->currentState()) {
    page.replace(F("%relaystate%"), F("<b style='color:red;'>ON</b>"));
  } else {
    page.replace(F("%relaystate%"), F("<b style='color:green;'>OFF</b>"));
  }

  if (relay->isDay(timeClient->getHours(), timeClient->getMinutes())) {
    page.replace(F("%isday%"), F("(actually run like day)"));
  } else {
    page.replace(F("%isday%"), F("(actually run like night)"));
  }
  
  //page += FPSTR(HTTP_PORTAL_OPTIONS);p
  page += FPSTR(HTTP_END);

  webServer->send(200, "text/html", page);
}

void Webserver::handleApplicationSettings() {
  DEBUG("Settings page");
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", _TERRASON_TITLE_);
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_APP_STYLE);
//  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += FPSTR(HTTP_APP_SETTINGS);
  page += FPSTR(HTTP_END);
  if (webServer->arg("submit") == "Save") {
    // Verify if all parameter is a integer  
    if (!isInteger(webServer->arg("daystarthour")) ||\
        !isInteger(webServer->arg("daystartminute")) ||
        !isInteger(webServer->arg("daystophour")) ||\
        !isInteger(webServer->arg("daystopminute")) ||\
        !isInteger(webServer->arg("daytemp")) ||\
        !isInteger(webServer->arg("nighttemp")) ||\
        !isInteger(webServer->arg("heatlessthan"))) {
          // Display error message
          page.replace("%message%", F("<b style='color:red;'>Please correct the error in fiel (that's integer fields)</b><br />"));
        } else {
          DEBUG("Backup application configuration asked by user.");
          // Backup parameters in the EEPROM
          page.replace("%message%", F("<b style='color:green;'>Settings saved into EEPROM memory.</b><br />"));
          appconfig.daystarthour = webServer->arg("daystarthour").toInt();
          appconfig.daystartminute = webServer->arg("daystartminute").toInt();
          appconfig.daystophour = webServer->arg("daystophour").toInt();
          appconfig.daystopminute = webServer->arg("daystopminute").toInt();
          appconfig.daytemp = webServer->arg("daytemp").toInt();
          appconfig.nighttemp = webServer->arg("nighttemp").toInt();
          appconfig.heatlessthan = webServer->arg("heatlessthan").toInt();
/*          byte* myptr = (byte *) &appconfig;
          for (int i=0; i<sizeof(appconfig); i++) {
            EEPROM.write(i, myptr[i]);
          } */
          EEPROM.put(0,appconfig);
          EEPROM.commit();
        }
    page.replace("%daystarthour%", webServer->arg("daystarthour"));
    page.replace("%daystartminute%", webServer->arg("daystartminute"));
    page.replace("%daystophour%", webServer->arg("daystophour"));
    page.replace("%daystopminute%", webServer->arg("daystopminute"));
    page.replace("%daytemp%", webServer->arg("daytemp"));
    page.replace("%nighttemp%", webServer->arg("nighttemp"));
    page.replace("%heatlessthan%", webServer->arg("heatlessthan"));
  } else {
    page.replace("%message%", "");
    page.replace("%daystarthour%", String(appconfig.daystarthour));
    page.replace("%daystartminute%", String(appconfig.daystartminute));
    page.replace("%daystophour%", String(appconfig.daystophour));
    page.replace("%daystopminute%", String(appconfig.daystopminute));
    page.replace("%daytemp%", String(appconfig.daytemp));
    page.replace("%nighttemp%", String(appconfig.nighttemp));
    page.replace("%heatlessthan%", String(appconfig.heatlessthan));
  }
  webServer->send(200, "text/html", page);
}

void Webserver::handleApplicationReset() {
  DEBUG("Reset page");
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", _TERRASON_TITLE_);
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_APP_STYLE);
  page += FPSTR(HTTP_HEAD_END);
  bool performreset = false;
  if (webServer->arg("performreset") == "") {
    page += FPSTR(HTTP_APP_RESET);
  } else {
    page += FPSTR(HTTP_APP_RESET_DONE);
    performreset = true;
    DEBUG(F("Reset device asked by user"));
  }

  if (webServer->arg("startota") == "start") {
    DEBUG(F("Start Arduino IDE developper OTA asked"));

    // Remove all that in a special web page
    ArduinoOTA.setHostname("TERRASON ESP8266");
    
    // No authentication by default
    ArduinoOTA.setPassword((const char *)"2309");
    
    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    page.replace("%messageota%","<b style='color:green'>OTA mode started, '2309'</b><br />");
  } else {
    page.replace("%messageota%","");
  }
  page += FPSTR(HTTP_END);
  webServer->send(200, "text/html", page);
  if (performreset) ESP.restart();
}

void Webserver::handleApplicationTemp() {
  String page = "";
  if (sensors->getMeasurement().tempvalid) page += sensors->getMeasurement().temp;
  webServer->send(200, "text/plain", page);
}

void Webserver::handleApplicationHum() {
  String page = "";
  if (sensors->getMeasurement().humvalid) page += sensors->getMeasurement().hum;
  webServer->send(200, "text/plain", page);
}

void Webserver::startSetupWifiPortal() {
  /*
   * Configure portal for wifi configuration
   */
  DEBUG(F("Configure HTTP portal setup wifi"));
  
  //webServer.reset(new ESP8266WebServer(80));

  // Start SERVER
  webServer->begin(); // Web server startº
  
  webServer->on("/", std::bind(&Webserver::handleAPRoot, this));
  webServer->on("/wifi", std::bind(&Webserver::handleAPWifi, this, true));
  webServer->on("/0wifi", std::bind(&Webserver::handleAPWifi, this, false));
  webServer->on("/wifisave", std::bind(&Webserver::handleAPWifiSave, this));
  webServer->on("/i", std::bind(&Webserver::handleAPInfo, this));
  //server->on("/generate_204", std::bind(&WiFiManager::handle204, this));  //Android/Chrome OS captive portal check.
  webServer->on("/fwlink", std::bind(&Webserver::handleAPRoot, this));  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  webServer->onNotFound (std::bind(&Webserver::handleAPNotFound, this));

  DEBUG(F("HTTP portal setup wifi started"));
}

/** Handle root or redirect to captive portal */
void Webserver::handleAPRoot() {
  DEBUG(F("Handle root"));
  if (captivePortal()) { // If caprive portal redirect instead of displaying the page.
    return;
  }

  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Options");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
//  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += "<h1>";
  page += F(_TERRASON_TITLE_);
  page += "</h1>";
  page += F("<h3></h3>");
  page += FPSTR(HTTP_PORTAL_OPTIONS);
  page += FPSTR(HTTP_END);

  webServer->send(200, "text/html", page);

}

/** Wifi config page handler */
void Webserver::handleAPWifi(boolean scan) {
  DEBUG("Handle Wifi");
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Config ESP");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_APP_STYLE);
//  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);

  if (scan) {
    int n = WiFi.scanNetworks();
    DEBUG(F("Scan done"));
    if (n == 0) {
      DEBUG(F("No networks found"));
      page += F("No networks found. Refresh to scan again.");
    } else {

      //sort networks
      int indices[n];
      for (int i = 0; i < n; i++) {
        indices[i] = i;
      }

      // RSSI SORT

      // old sort
      for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
          if (WiFi.RSSI(indices[j]) > WiFi.RSSI(indices[i])) {
            std::swap(indices[i], indices[j]);
          }
        }
      }

      /*std::sort(indices, indices + n, [](const int & a, const int & b) -> bool
        {
        return WiFi.RSSI(a) > WiFi.RSSI(b);
        });*/

      // remove duplicates ( must be RSSI sorted )
/*      if (_removeDuplicateAPs) {
        String cssid;
        for (int i = 0; i < n; i++) {
          if (indices[i] == -1) continue;
          cssid = WiFi.SSID(indices[i]);
          for (int j = i + 1; j < n; j++) {
            if (cssid == WiFi.SSID(indices[j])) {
              DEBUG_WM("DUP AP: " + WiFi.SSID(indices[j]));
              indices[j] = -1; // set dup aps to index -1
            }
          }
        }
      } */

      page += "<form method='get' action='wifisave'><table>";
      //display networks in page
      for (int i = 0; i < n; i++) {
        if (indices[i] == -1) continue; // skip dups
        DEBUG(WiFi.SSID(indices[i]));
        DEBUG(WiFi.RSSI(indices[i]));
        int quality = getRSSIasQuality(WiFi.RSSI(indices[i]));

        if (_minimumQuality == -1 || _minimumQuality < quality) {
          String item = FPSTR(HTTP_WIFI_ITEM);
          String rssiQ;
          rssiQ += quality;
          item.replace("{v}", WiFi.SSID(indices[i]));
          item.replace("{r}", rssiQ);
          if (WiFi.encryptionType(indices[i]) != ENC_TYPE_NONE) {
            item.replace("{i}", "l");
          } else {
            item.replace("{i}", "");
          }
          //DEBUG_WM(item);
          page += item;
          delay(0);
        } else {
          DEBUG(F("Skipping due to quality"));
        }

      }
      page += "</table></form>";
      page += "<br/>";
    }
  }

  page += FPSTR(HTTP_WIFI_FORM);
  char parLength[2];
  // add the extra parameters to the form

/*  
  for (int i = 0; i < _paramsCount; i++) {
    if (_params[i] == NULL) {
      break;
    }

    String pitem = FPSTR(HTTP_FORM_PARAM);
    if (_params[i]->getID() != NULL) {
      pitem.replace("{i}", _params[i]->getID());
      pitem.replace("{n}", _params[i]->getID());
      pitem.replace("{p}", _params[i]->getPlaceholder());
      snprintf(parLength, 2, "%d", _params[i]->getValueLength());
      pitem.replace("{l}", parLength);
      pitem.replace("{v}", _params[i]->getValue());
      pitem.replace("{c}", _params[i]->getCustomHTML());
    } else {
      pitem = _params[i]->getCustomHTML();
    }

    page += pitem;
  }
  if (_params[0] != NULL) {
    page += "<br/>";
  } 
*/
/*
  if (_sta_static_ip) {

    String item = FPSTR(HTTP_FORM_PARAM);
    item.replace("{i}", "ip");
    item.replace("{n}", "ip");
    item.replace("{p}", "Static IP");
    item.replace("{l}", "15");
    item.replace("{v}", _sta_static_ip.toString());

    page += item;

    item = FPSTR(HTTP_FORM_PARAM);
    item.replace("{i}", "gw");
    item.replace("{n}", "gw");
    item.replace("{p}", "Static Gateway");
    item.replace("{l}", "15");
    item.replace("{v}", _sta_static_gw.toString());

    page += item;

    item = FPSTR(HTTP_FORM_PARAM);
    item.replace("{i}", "sn");
    item.replace("{n}", "sn");
    item.replace("{p}", "Subnet");
    item.replace("{l}", "15");
    item.replace("{v}", _sta_static_sn.toString());

    page += item;

    page += "<br/>";
  }
*/
  page += FPSTR(HTTP_FORM_END);
  page += FPSTR(HTTP_SCAN_LINK);

  page += FPSTR(HTTP_END);

  webServer->send(200, "text/html", page);


  DEBUG(F("Sent config page"));
}

/** Handle the WLAN save form and redirect to WLAN config page again */
void Webserver::handleAPWifiSave() {
  DEBUG(F("WiFi save"));

  //SAVE/connect here
  _ssid = webServer->arg("s").c_str();
  _pass = webServer->arg("p").c_str();

  //parameters
/*  for (int i = 0; i < _paramsCount; i++) {
    if (_params[i] == NULL) {
      break;
    }
    //read parameter
    String value = server->arg(_params[i]->getID()).c_str();
    //store it in array
    value.toCharArray(_params[i]->_value, _params[i]->_length);
    DEBUG(F("Parameter"));
    DEBUG(_params[i]->getID());
    DEBUG(value);
  } */


/*  
  if (server->arg("ip") != "") {
    DEBUG(F("static ip"));
    DEBUG(server->arg("ip"));
    //_sta_static_ip.fromString(server->arg("ip"));
    String ip = server->arg("ip");
    optionalIPFromString(&_sta_static_ip, ip.c_str());
  }
  if (server->arg("gw") != "") {
    DEBUG(F("static gateway"));
    DEBUG(server->arg("gw"));
    String gw = server->arg("gw");
    optionalIPFromString(&_sta_static_gw, gw.c_str());
  }
  if (server->arg("sn") != "") {
    DEBUG(F("static netmask"));
    DEBUG(server->arg("sn"));
    String sn = server->arg("sn");
    optionalIPFromString(&_sta_static_sn, sn.c_str());
  }
*/
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Credentials Saved");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_APP_STYLE);
//  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += FPSTR(HTTP_SAVED);
  page += FPSTR(HTTP_END);

  webServer->send(200, "text/html", page);

  DEBUG(F("Sent wifi save page"));

  //connect = true; //signal ready to connect/reset
}

/** Handle the info page */
void Webserver::handleAPInfo() {
  DEBUG(F("Info"));

  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Info");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_APP_STYLE);
//  page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
/*  page += F("<dl>");
  page += F("<dt>Chip ID</dt><dd>");
  page += ESP.getChipId();
  page += F("</dd>");
  page += F("<dt>Flash Chip ID</dt><dd>");
  page += ESP.getFlashChipId();
  page += F("</dd>");
  page += F("<dt>IDE Flash Size</dt><dd>");
  page += ESP.getFlashChipSize();
  page += F(" bytes</dd>");
  page += F("<dt>Real Flash Size</dt><dd>");
  page += ESP.getFlashChipRealSize();
  page += F(" bytes</dd>");
  page += F("<dt>Soft AP IP</dt><dd>");
  page += WiFi.softAPIP().toString();
  page += F("</dd>");
  page += F("<dt>Soft AP MAC</dt><dd>");
  page += WiFi.softAPmacAddress();
  page += F("</dd>");
  page += F("<dt>Station MAC</dt><dd>");
  page += WiFi.macAddress();
  page += F("</dd>");
  page += F("</dl>"); */
  page += FPSTR(HTTP_APP_DEVICE_INFO);
  page.replace("%chipid%", String(ESP.getChipId()));
  page.replace("%flashchipid%", String(ESP.getFlashChipId()));  
  page.replace("%ideflashsize%", String(ESP.getFlashChipSize()));
  page.replace("%realflashsize%", String(ESP.getFlashChipRealSize()));
  page.replace("%softapip%", WiFi.softAPIP().toString());
  page.replace("%softapmac%", WiFi.softAPmacAddress());
  page.replace("%stationmac%", WiFi.macAddress());

  page += FPSTR(HTTP_END);

  webServer->send(200, "text/html", page);

  DEBUG(F("Sent info page"));
}

void Webserver::handleAPNotFound() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
    return;
  }
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer->uri();
  message += "\nMethod: ";
  message += ( webServer->method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webServer->args();
  message += "\n";

  for ( uint8_t i = 0; i < webServer->args(); i++ ) {
    message += " " + webServer->argName ( i ) + ": " + webServer->arg ( i ) + "\n";
  }
  webServer->sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  webServer->sendHeader("Pragma", "no-cache");
  webServer->sendHeader("Expires", "-1");
  webServer->send ( 404, "text/plain", message );
}


/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean Webserver::captivePortal() {  if (!isIp(webServer->hostHeader()) ) {
    DEBUG(F("Request redirected to captive portal"));
    webServer->sendHeader("Location", String("http://") + toStringIp(webServer->client().localIP()), true);
    webServer->send ( 302, "text/plain", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves.
    webServer->client().stop(); // Stop is needed because we sent no content length
    return true;
  }
  return false;
}

void Webserver::loop() {
  webServer->handleClient();
  ArduinoOTA.handle();
}

int Webserver::getRSSIasQuality(int RSSI) {
  int quality = 0;

  if (RSSI <= -100) {
    quality = 0;
  } else if (RSSI >= -50) {
    quality = 100;
  } else {
    quality = 2 * (RSSI + 100);
  }
  return quality;
}

boolean Webserver::isIp(String str) {
  for (int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}

String Webserver::toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

bool Webserver::isInteger(String str) {
  for (int i=0; i<str.length();i++) {
    if (!isdigit(str[i])) return false;
  };
  return true;
}


template <typename Generic>
void Webserver::DEBUG(Generic text) {
  if (this->_debug) {
    Serial.print("*WEB: ");
    Serial.println(text);
  }
}
