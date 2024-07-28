
// --------------- DECRYPTION KEY ---------------

// The decryption key should be provided as byte array (i.e "7EA0" -> {0x7e, 0xa0})
const byte KEY[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// --------------- MQTT ---------------

const bool MQTT_ENABLED = true;
const char* WIFI_SSID = "ssid";
const char* WIFI_PASS = "pass";
const char* MQTT_SERVER = "127.0.0.1";
const int MQTT_PORT = 1883;
const char* MQTT_USER = "user";
const char* MQTT_PASS = "pass";
const char* MQTT_TOPIC = "homeassistant/sensor/smartmeter/state";

// --------------- SMART METER ---------------

HardwareSerial *smart_meter = &Serial;
const int SMARTMETER_BAUD_RATE = 9600;
// #define SWAP_SERIAL  // Use alternative pins for Serial: GPIO15 (TX) and GPIO13 (RX) 

// --------------- LOGGING ---------------

// Comment out or delete the next two lines to disable logging
#define LOGGING_ENABLED
const int SERIAL_MONITOR_BAUD_RATE = SMARTMETER_BAUD_RATE;  // because we use the same port
