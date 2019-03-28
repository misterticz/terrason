# Terrason
Sonoff TH10/16 Vivarium temp control for alive animals like snake or others.

The project is based on SONOFF TH10 and TH16 product.
- Developped on TH10 rev 2.1
- Support SONOFF temperature sensor DS18B20
- Support SONOFF temp/hum sensor SI7021

The project keep clock updated using NTP.
Terrason offer a webservice available on his ip address on the lan network.

The code of the wifi library is based on the code of WifiManager of
Code for read SONOFF SI7021 sensors is based on the code of DHT plugin of ESPEasy.

### How to use

- Flash the terrason .bin firmware in TH10 or TH16 (or flash with arduino ide)
- A long press (more than 5 seconds) put terrason in association mode
- Connect to terrason wifi and enter your wifi credentials
- After restarting connect you on the ip of the terrason/sonoff device

### Get the last digit of the IP address

When a client connection is successfull, the blue red tell 3 digits:

flash count for each digit or a long flash for zero:

exemple: . _ . mean 101 so certainly "192.168.0.101" or "192.168.1.101".
Check your network configuration to know that.
You can successfully fixe the IP of the device in your router, it prevent ip adress changement.

### Blue led

- Fixed : Wifi connected, one sensor connected
- Fast blinking : Wifi disconnected
- Hard blinking : Sensor disconnected
- Off : it's not terrason mode

### Button

- Short press : tell the IP address (if available) on blue led
- Long press more than 5 secondes : put the device in assoc mode
- Short press in assoc mode: stop the association wifi AP

### Red led / Relay

The led is automatically lighting when relay is ON.
BY SECURITY, THE LED/RELAY IS ALWAYS OFF IF THE SENSORS ISN'T
CORRECTLY RECOGNISED FROM MAX ABOUT 10 SECONDS.

Relay is closed (heating) when:
- sensors is offline, no temp measurement
- in day : temp < day_temp - healessthan
- in night : temp < night_temp - heatlessthan

Relay is open (no heating) when:
- temp >= day_temp or day_night
