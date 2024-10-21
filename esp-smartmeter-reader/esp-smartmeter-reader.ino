#include "config.h"
#include <FastCRC.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#if defined(ESP32)
#include "mbedtls/aes.h"
#include <WiFi.h>
#elif defined(ESP8266)
#include <AES.h>
#include <CTR.h>
#include <ESP8266WiFi.h>
#else
#error "Error: The device is not a ESP8266 or ESP32!"
#endif

// --------------- WIFI ---------------

void SetupWifi() {
  delay(10);

  logger->println();
  logger->print("Connecting to ");
  logger->println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    logger->print(".");
  }

  logger->println("");
  logger->println("WiFi connected");
  logger->println("IP address: ");
  logger->println(WiFi.localIP());
}

// --------------- MQTT ---------------

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

void ReconnectMQTT() {
  while (!mqtt_client.connected()) {
    logger->print("Attempting MQTT connection...");
    if (mqtt_client.connect("ESPClient", MQTT_USER, MQTT_PASS)) {
      logger->println("connected");
    } else {
      logger->print("failed, rc=");
      logger->print(mqtt_client.state());
      logger->println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// --------------- SERIAL READER ---------------


byte received_data[MESSAGE_LENGTH];

void ReadSerialData() {
  static byte previous_byte = 0;
  static bool receiving = false;
  static unsigned int pos;

  while (smart_meter->available() > 0) {
    byte current_byte = smart_meter->read();
    if (!receiving) {
      // Starting sequence is 7E A0
      if (previous_byte == 0x7E && current_byte == 0xA0) {
        receiving = true;
        received_data[0] = 0x7E;
        received_data[1] = 0xA0;
        pos = 2;
      }
    } else {
      if (pos < MESSAGE_LENGTH) {
        received_data[pos] = current_byte;
        pos++;
      } else {
        ParseReceivedData();
        receiving = false;
      }
    }
    previous_byte = current_byte;
  }
}

// -------------- DATA PARSER ---------------

int BytesToInt(byte bytes[], unsigned int left, unsigned int right) {
  int result = 0;
  for (unsigned int i = left; i < right; i++) {
    result = result * 256 + bytes[i];
  }
  return result;
}

void ParseReceivedData() {
  // Discard message if CRC check fails
  if (!ValidateCRC()) {
    return;
  }

  // Decrypt message
  byte decrypted_message[PAYLOAD_LENGTH];
  DecryptMessage(decrypted_message);

  // Extract time and date from decrypted message
  int year = BytesToInt(decrypted_message, 22 + PAYLOAD_ADD, 24 + PAYLOAD_ADD);
  int month = BytesToInt(decrypted_message, 24 + PAYLOAD_ADD, 25 + PAYLOAD_ADD);
  int day = BytesToInt(decrypted_message, 25 + PAYLOAD_ADD, 26 + PAYLOAD_ADD);
  int hour = BytesToInt(decrypted_message, 27 + PAYLOAD_ADD, 28 + PAYLOAD_ADD);
  int minute = BytesToInt(decrypted_message, 28 + PAYLOAD_ADD, 29 + PAYLOAD_ADD);
  int second = BytesToInt(decrypted_message, 29 + PAYLOAD_ADD, 30 + PAYLOAD_ADD);
  char timestamp[20];
  sprintf(timestamp, "%02d.%02d.%04d %02d:%02d:%02d", day, month, year, hour, minute, second);

  // Create JSON
  StaticJsonDocument<256> doc;
  doc["timestamp"] = timestamp;
  doc["+A"] = BytesToInt(decrypted_message, 35 + PAYLOAD_ADD, 39 + PAYLOAD_ADD)/1000.0;
  doc["-A"] = BytesToInt(decrypted_message, 40 + PAYLOAD_ADD, 44 + PAYLOAD_ADD)/1000.0;
  doc["+R"] = BytesToInt(decrypted_message, 45 + PAYLOAD_ADD, 49 + PAYLOAD_ADD)/1000.0;
  doc["-R"] = BytesToInt(decrypted_message, 50 + PAYLOAD_ADD, 54 + PAYLOAD_ADD)/1000.0;
  doc["+P"] = BytesToInt(decrypted_message, 55 + PAYLOAD_ADD, 59 + PAYLOAD_ADD);
  doc["-P"] = BytesToInt(decrypted_message, 60 + PAYLOAD_ADD, 64 + PAYLOAD_ADD);
  doc["+Q"] = BytesToInt(decrypted_message, 65 + PAYLOAD_ADD, 69 + PAYLOAD_ADD);
  doc["-Q"] = BytesToInt(decrypted_message, 70 + PAYLOAD_ADD, 74 + PAYLOAD_ADD);
  char payload[256];
  serializeJson(doc, payload);

  // Publish JSON
  logger->println(payload);
  if (MQTT_ENABLED) {
    mqtt_client.publish(MQTT_TOPIC, payload, false);
  }
}

// --------------- CRC VALIDATOR ---------------

FastCRC16 CRC16;

bool ValidateCRC() {
  byte message[MESSAGE_LENGTH - 4];
  memcpy(message, received_data + 1, MESSAGE_LENGTH - 4);
  int crc = CRC16.x25(message, MESSAGE_LENGTH - 4);
  int expected_crc = received_data[MESSAGE_LENGTH - 2] * 256 + received_data[MESSAGE_LENGTH - 3];
  if (crc != expected_crc) {
    logger->println("WARNING: CRC check failed");
    return false;
  }
  return true;
}

// --------------- DECRYPTOR ---------------

#if defined(ESP32)
mbedtls_aes_context aes;
#elif defined(ESP8266)
CTR<AES128> ctr;
#endif

void DecryptMessage(byte decrypted_message[PAYLOAD_LENGTH]) {
  // Extract message and iv from received data
  byte encrypted_message[PAYLOAD_LENGTH] = {0};
  memcpy(encrypted_message, received_data + 28 + IV_ADD, PAYLOAD_LENGTH);
  byte iv[16] = {0};
  memcpy(iv, received_data + 14 + IV_ADD, 8);
  memcpy(iv + 8, received_data + 24 + IV_ADD, 4);
  iv[15] = 0x02;

  // Decrypt message
  #if defined(ESP32)
  size_t nc_off = 0;
  unsigned char stream_block[16] = {0};
  mbedtls_aes_crypt_ctr(&aes, PAYLOAD_LENGTH, &nc_off, iv, stream_block, encrypted_message, decrypted_message);
  #elif defined(ESP8266)
  ctr.setIV(iv, sizeof(iv));
  ctr.decrypt(decrypted_message, encrypted_message, sizeof(encrypted_message));
  #endif
}

// --------------- SETUP & LOOP ---------------

void setup() {
  smart_meter->begin(SMARTMETER_BAUD_RATE);
  #ifdef SWAP_SERIAL
    smart_meter->swap();  // ESP8266: Remap Serial (UART0) to GPIO15 (TX) and GPIO13 (RX)
  #endif
  logger->begin(SERIAL_MONITOR_BAUD_RATE);
  #if defined(ESP32)
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, KEY, 128);
  #elif defined(ESP8266)
  ctr.setKey(KEY, sizeof(KEY));
  #endif
  if (MQTT_ENABLED) {
    SetupWifi();
    mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
  }
}

void loop() {
  if (MQTT_ENABLED) {
    if (!mqtt_client.connected()) {
      ReconnectMQTT();
    }
    mqtt_client.loop();
  }
  ReadSerialData();
}
