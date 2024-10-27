# ESP32 / ESP8266 Smart Meter Reader for Wiener Netze

[![Compile Sketch for ESP32](https://github.com/aldadic/esp-smartmeter-reader/actions/workflows/compile_esp32.yml/badge.svg)](https://github.com/aldadic/esp-smartmeter-reader/actions/workflows/compile_esp32.yml)
[![Compile Sketch for ESP8266](https://github.com/aldadic/esp-smartmeter-reader/actions/workflows/compile_esp8266.yml/badge.svg)](https://github.com/aldadic/esp-smartmeter-reader/actions/workflows/compile_esp8266.yml)

This Arduino sketch was made to read and decrypt the data from my smart meter operated by Wiener Netze and publish it via MQTT. This way the data can be easily integrated into Home Assistant (see [here](#home-assistant-integration)). This sketch works both on the ESP32 and ESP8266. This project was only possible with the information from [this thread](https://www.lteforum.at/mobilfunk/wiener-netze-smart-meter-auslesen.16222/); thank you to everyone who shared their findings (especially the user "pocki").

## Supported smart meters

The sketch supports the following smart meter manufacturers:

* Landis+Gyr (tested with Landis+Gyr E450)
* Siemens (thanks [@nomike](https://github.com/nomike))

## Setting up the ESP32 / ESP8266

To be able to use the optical user interface of the smart meter an [IR reader with TTL-Interface](https://wiki.volkszaehler.org/hardware/controllers/ir-schreib-lesekopf-ttl-ausgang) is needed (I am using [this](https://www.ebay.de/itm/313460034498) one). This device reads the data from the optical user interface and transmits it to the ESP32 or ESP8266 via serial communication. Because normally UART0 is used for the USB connection (e.g. flashing the device, serial monitor, etc.), it makes sense to use UART2 for the IR reader. This is why we connect the IR reader to the ESP32 in the following way:

| IR reader  | ESP32        |
| ---------- | ------------ |
| VCC        | 3.3V         |
| GND        | GND          |
| RX         | GPIO16 (RX2) |
| TX         | GPIO17 (TX2) |

The ESP8266 has two UARTs, however, UART1 is TX-only. Therefore, we have to use UART0 for the IR reader. In order to not interfere with the USB connection, we use `Serial.swap()` (by defining `SWAP_SERIAL` in the config file) to swap the pins for UART0 to GPIO15 (TX) and GPIO13 (RX). However, this way we can't use UART0 for logging anymore. But since we only want to send data to the serial monitor, we can use UART1 for that and still send the data through the USB connection by connecting GPIO2 (D4) to GPIO1 (TX). Hence, we connect the IR reader to the ESP8266 in the following way:

| IR reader  | ESP8266                          |
| ---------- | -------------------------------- |
| VCC        | 3.3V                             |
| GND        | GND                              |
| RX         | GPIO13 (D7)                      |
| TX         | GPIO15 (D8)                      |
|            | connect GPIO2 (D4) to GPIO1 (TX) |

## Configuring and uploading the sketch

After you cloned the repository, rename `config_esp32.h` or `config_esp8266.h` (depending on which device you have) to `config.h` and open it. Adjust the settings and save the file (the decryption key can be obtained in the [Smart Meter Webportal](https://smartmeter-web.wienernetze.at/)). Don't forget to uncomment the settings for your specific smart meter brand in the "SMART METER" section. After that open `esp-smartmeter-reader.ino` in the Arduino IDE. The sketch uses the following libaries:

* [FastCRC](https://github.com/FrankBoesing/FastCRC)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [PubSubClient](https://github.com/knolleary/pubsubclient)
* [Crypto](https://github.com/OperatorFoundation/Crypto) (only for ESP8266*)

All those libraries must be installed via the Library Manager. Once that is done you should be able to compile and upload the sketch via the Arduino IDE.

\*For the ESP32, we use the [mbedtls](https://github.com/Mbed-TLS/mbedtls) library for decryption, which should be already available in the Arduino IDE. Because mbedtls is not available for the ESP8266, we use the [Crypto](https://github.com/OperatorFoundation/Crypto) library on the ESP8266 instead.

## Home Assistant integration

The example below shows how the data can be integrated into Home Assistant:

```yaml
mqtt:
  sensor:
    - name: "Zählerstand"
      state_topic: "homeassistant/sensor/smartmeter/state"  # use topic from config.h
      device_class: "energy"
      unit_of_measurement: "kWh"
      state_class: "total_increasing"
      value_template: "{{ value_json['+A'] }}"
    - name: "Momentanleistung"
      state_topic: "homeassistant/sensor/smartmeter/state" # use topic from config.h
      device_class: "power"
      unit_of_measurement: "W"
      state_class: "measurement"
      value_template: "{{ value_json['+P'] }}"

# the history of instantaneous power is not interesting 
# therefore exclude it from the recorder
recorder:
  exclude:
    entities:
      - sensor.momentanleistung
```
