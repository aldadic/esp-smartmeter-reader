# ESP32 / ESP8266 Smart Meter Reader for Wiener Netze

This Arduino sketch was made to read and decrypt the data from my Smart Meter (Landis+Gyr E450 operated by Wiener Netze) and publish it via MQTT. This way the data can be easily integrated into Home Assistant (see [here](#home-assistant-integration)). This sketch works both on the ESP32 and ESP8266. This project was only possible with the information from [this thread](https://www.lteforum.at/mobilfunk/wiener-netze-smart-meter-auslesen.16222/); thank you to everyone who shared their findings (especially the user "pocki").

## Setting up the ESP32 / ESP8266

To be able to use the optical user interface of the Smart Meter an [IR reader with TTL-Interface](https://wiki.volkszaehler.org/hardware/controllers/ir-schreib-lesekopf-ttl-ausgang) is needed (I am using [this](https://www.ebay.de/itm/313460034498) one). This device reads the data from the optical user interface and transmits it to the ESP32 or ESP8266 via serial communication. Because normally UART0 is used for the USB connection (e.g. flashing the device, serial monitor, etc.), it makes sense to use UART2 for the IR meter to avoid any interference. This is why we connect the IR reader to the ESP32 in the following way:

| IR reader  | ESP32        |
| ---------- | ------------ |
| VCC        | 3.3V         |
| GND        | GND          |
| RX         | GPIO16 (RX2) |
| TX         | GPIO17 (TX2) |

Since only UART0 is available on the ESP8266, we have to connect the IR reader to the ESP8266 in this way:

| IR reader  | ESP8266      |
| ---------- | ------------ |
| VCC        | 3.3V         |
| GND        | GND          |
| RX         | GPIO3 (RX)   |
| TX         | GPIO1 (TX)   |

However, this causes some problems. First of all, you will need to disconnect the IR reader from the ESP8266 while uploading the sketch. Furthermore, using the serial monitor for logging purposes might cause issues. It works for me, but you should consider disabling logging if you encounter any problems.

## Configuring and uploading the sketch

After you cloned the repository, rename `config_esp32.h` or `config_esp8266.h` (depending on which device you have) to `config.h` and open it. Adjust the settings and save the file (the decryption key can be obtained in the [Smart Meter Webportal](https://smartmeter-web.wienernetze.at/)). After that open `esp-smartmeter-reader.ino` in the Arduino IDE. The sketch uses the following libaries:

* [FastCRC](https://github.com/FrankBoesing/FastCRC)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [PubSubClient](https://github.com/knolleary/pubsubclient)

All those libraries must be installed via the Library Manager. For the ESP32 we will use the [mbedtls](https://github.com/Mbed-TLS/mbedtls) library for decryption, which should be already available in the Arduino IDE. Because mbedtls is not available for the ESP8266, we use the [Crypto](https://github.com/OperatorFoundation/Crypto) library on the ESP8266 instead. Therefore, you must also install this library via the Library Manager if you are using the ESP8266. Once that is done you should be able to compile and upload the sketch via the Arduino IDE.

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

## TODO

It would be nice to adapt this sketch for [ESPHome](https://esphome.io/). This should be possible with the [Custom UART Device](https://esphome.io/custom/uart.html) and [this guide](https://esphome.io/components/sensor/custom.html).
