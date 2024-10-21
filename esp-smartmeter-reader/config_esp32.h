
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

HardwareSerial *smart_meter = &Serial2;
const int SMARTMETER_BAUD_RATE = 9600;

// // Landys & Gyr
// const unsigned int IV_ADD = 0;
// const unsigned int PAYLOAD_ADD = 0;
// const unsigned int MESSAGE_LENGTH = 105;
// const unsigned int PAYLOAD_LENGTH = 74;

// // Siemens
// const unsigned int IV_ADD = 2;
// const unsigned int PAYLOAD_ADD = 18;
// const unsigned int MESSAGE_LENGTH = 125; // Landys & Gyr: 105 ; 
// const unsigned int PAYLOAD_LENGTH = 90; // 74

// --------------- LOGGING ---------------

HardwareSerial *logger = &Serial;
const int SERIAL_MONITOR_BAUD_RATE = 9600;
