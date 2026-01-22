// Custom serial configuration for FDRS to use Serial2 (PortC) on ESP32
// This header should be included before fdrs_gateway.h to override default UART_IF

#ifndef FDRS_CUSTOM_SERIAL_H
#define FDRS_CUSTOM_SERIAL_H

// Define a flag to indicate we're using custom serial
#define CUSTOM_SERIAL_CONFIG

// Include ArduinoJson before redefining UART_IF
#include <ArduinoJson.h>

// Override default UART_IF for ESP32 to use Serial2 (PortC) instead of Serial1
#if defined (ESP32)
  #if !defined RXD2 or !defined TXD2
      #warning Defining RXD2 and TXD2 using MCU defaults.
      #if CONFIG_IDF_TARGET_ESP32
          #define RXD2 13
          #define TXD2 14
      #elif CONFIG_IDF_TARGET_ESP32S2 or CONFIG_IDF_TARGET_ESP32S3
          #define RXD2 18
          #define TXD2 17
      #elif CONFIG_IDF_TARGET_ESP32C3
          #define RXD2 2
          #define TXD2 3
      #else
          #error MCU not supported.
      #endif
  #endif
#endif

// Redefine UART_IF to use Serial2 for PortC
#ifdef UART_IF
#undef UART_IF
#endif
#define UART_IF Serial2  // Use Serial2 for PortC instead of Serial1

// Include time functionality
#include <time.h>

extern time_t now;

// External variables required by the serial functions
extern DataReading theData[256];
extern uint8_t ln;
extern uint8_t newData;
extern TimeSource timeSource;

// GPS defines if needed
#ifdef USE_GPS
  #define GPS_IF Serial1  // Use Serial1 for GPS if needed
#endif

// Custom serial handling functions that will work with Serial2
void getSerialCustom() {
  String incomingString;

  // Check if data is available on Serial2 (PortC)
  if (UART_IF.available()) {
    incomingString = UART_IF.readStringUntil('\n');
    incomingString.trim(); // Remove any trailing whitespace
  }
  else if (Serial.available()) {
    incomingString = Serial.readStringUntil('\n');
    incomingString.trim(); // Remove any trailing whitespace
  }

#ifdef GPS_IF
  if (GPS_IF.available()) {
    // Data is coming in every second from the GPS, let's minimize the processing power
    // required by only parsing periodically - maybe every 60 seconds.
    static unsigned long lastGpsParse = 0;
    if(lastGpsParse == 0 || TDIFFSEC(lastGpsParse,60)) {
      lastGpsParse = millis();
      for(int i=0; i < 20; i++) {
        incomingString = GPS_IF.readStringUntil('\n');
        // Process GPS data if needed
      }
    }
    return;
  }
#endif // GPS_IF

  // Only process if we have data
  if (incomingString.length() > 0) {
    // Parse the incoming JSON data
    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, incomingString);
    if (error) {    // Test if parsing succeeds.
      DBG2("JSON parse error: " + incomingString);
      return;
    } else {
      int s = doc.size();
      JsonObject obj = doc[0].as<JsonObject>();
      if(obj["type"].is<uint8_t>()) { // DataReading
        for (int i = 0; i < s; i++) {
          theData[i].id = doc[i]["id"];
          theData[i].t = doc[i]["type"];
          theData[i].d = doc[i]["data"];
        }
        ln = s;
        newData = event_serial;
        DBG("Incoming Serial from PortC");
        String data;
        serializeJson(doc, data);
        DBG1("DR data: " + data);
      }
      else if(obj["cmd"].is<uint32_t>()) { // SystemPacket
        // Handle system commands if needed
      }
      else {
        DBG2("Incoming Serial: unknown format: " + incomingString);
      }
    }
  }
}

void handleSerialCustom() {
  // Handle data from Serial2 (PortC) and Serial
  // 限制每次处理的数据量，防止阻塞
  int processedCount = 0;
  const int MAX_PER_LOOP = 10; // 每次循环最多处理10条消息
  
  // Process only available data without blocking
  while (UART_IF.available() && processedCount < MAX_PER_LOOP) {
    getSerialCustom();
    yield(); // Allow other processes to run
    processedCount++;
  }
  
  processedCount = 0; // 重置计数器
  while (Serial.available() && processedCount < MAX_PER_LOOP) {
    getSerialCustom();
    yield(); // Allow other processes to run
    processedCount++;
  }
}

#endif // FDRS_CUSTOM_SERIAL_H