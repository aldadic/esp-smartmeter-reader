
// --------------- DECRYPTION KEY ---------------

// The decryption should be provided as byte array (i.e "7EA0" -> {0x7e, 0xa0})
const byte KEY[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// --------------- MQTT ---------------

const bool MQTT_ENABLED = false;
const char* WIFI_SSID = "ssid";
const char* WIFI_PASS = "pass";
const char* MQTT_SERVER = "127.0.0.1";
const int MQTT_PORT = 1883;
const char* MQTT_USER = "user";
const char* MQTT_PASS = "pass";
const char* MQTT_TOPIC = "homeassistant/sensor/smartmeter/state";

// --------------- SERIAL PORTS ---------------

const int CONSOLE_BAUD_RATE = 9600;
const int SMARTMETER_BAUD_RATE = 9600;
HardwareSerial *console = &Serial;
HardwareSerial *smart_meter = &Serial2;