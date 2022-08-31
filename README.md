# ESP32 Smart Meter Reader for Wiener Netze

This Arduino sketch was made to read and decrypt the data from my Smart Meter (Landis+Gyr E450 operated by Wiener Netze) and publish it via MQTT. This way the data can be easily integrated into Home Assistant (see [here](#home-assistant-integration)). Although this sketch was made for the ESP32, it should also work with the ESP8266 after minor adjustments (more on that later). This project was only possible with the information from [this thread](https://www.lteforum.at/mobilfunk/wiener-netze-smart-meter-auslesen.16222/); thank you to everyone who shared their findings (especially the user "pocki").

## Setting up the ESP32

To be able to use the optical user interface of the Smart Meter an [IR reader with TTL-Interface](https://wiki.volkszaehler.org/hardware/controllers/ir-schreib-lesekopf-ttl-ausgang) is needed (I am using [this](https://www.ebay.de/itm/313460034498) one). This device reads the data from the optical user interface and transmits it to the ESP32 via serial communication. Since I wanted to use UART2 for this, my IR reader is connected to my ESP32 in the following way:

| IR reader  | ESP32        |
| ---------- | ------------ |
| VCC        | 3.3V         |
| GND        | GND          |
| RX         | GPIO16 (RX2) |
| TX         | GPIO17 (TX2) |

If you want to use a different serial port or an ESP8266 instead of the ESP32 (the ESP8266 has only UART0 available), you will need to change the wiring and the serial port that is used by the sketch. This is done in the `config.h` file (see [here](#configuring-and-uploading-the-sketch)).

## Configuring and uploading the sketch

After you cloned the repository, rename the `config_example.h` file to `config.h` and open it. Adjust the settings and save the file (the decryption key can be obtained in the [Smart Meter Webportal](https://smartmeter-web.wienernetze.at/)). After that open `esp32-smartmeter-reader.ino` in the Arduino IDE. The sketch uses the following libaries:

* [mbedtls](https://github.com/Mbed-TLS/mbedtls)
* [FastCRC](https://github.com/FrankBoesing/FastCRC)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [PubSubClient](https://github.com/knolleary/pubsubclient)

mbedtls should be already available in the Arduino IDE. All other libraries must be installed via the Library Manager. Once that is done, you should be able to upload the sketch via the Arduino IDE.

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
