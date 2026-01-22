// Custom FDRS Gateway implementation for UART to MQTT functionality
#ifndef FDRS_CUSTOM_GATEWAY_H
#define FDRS_CUSTOM_GATEWAY_H

// Include required headers
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "fdrs_gateway_config.h"
#include <fdrs_datatypes.h>
#include <fdrs_debug.h>

// Global variables (must be declared before including components that use them)
SystemPacket theCmd;
TimeSource timeSource; // Define the timeSource variable first
DataReading theData[256];
uint8_t ln;
uint8_t newData = event_clear;
uint8_t newCmd = cmd_clear;

DataReading fdrsData[256]; // buffer for loadFDRS()
uint8_t data_count = 0;

// Include our custom serial configuration before other FDRS components
#include "fdrs_custom_serial.h"

// Define MQTT client
extern WiFiClient espClient;
WiFiClient espClient;
PubSubClient client(espClient);

// Time-related variables (to avoid including fdrs_time.h which has dependencies)
time_t now;                           // Current time in UTC - number of seconds since Jan 1 1970 (epoch)
struct tm timeinfo;                   // Structure containing time elements
bool validTimeFlag = false;           // Indicate whether we have reliable time 
bool validRtcFlag = false;            // Is RTC date and time valid?
bool isDST = false;                           // Keeps track of Daylight Savings Time vs Standard Time
long slewSecs = 0;                  // When time is set this is the number of seconds the time changes

// Define the time offset constants from the config
double stdOffset = (STD_OFFSET * 60 * 60);  // UTC -> Local time, in Seconds, offset from UTC in Standard Time
double dstOffset = (DST_OFFSET * 60 * 60); // -1 hour for DST offset from standard time (in seconds)
time_t lastDstCheck = 0;
unsigned long lastTimeSend = 0;
unsigned long lastRtcCheck = 0;
unsigned long lastRtcTimeSetMin = 0;

// Time macros
#define MIN_TS 1718000000 // Time in Unit timestamp format should be greater than this number to be valid
#define MAX_TS 3318000000 // time in Unit timestamp format should be less than this number to be valid
#define VALID_TS(_unixts) ( (_unixts > MIN_TS && _unixts < MAX_TS) ? true : false )
#define TDIFF(prevMs,durationMs) (millis() - prevMs > durationMs)
#define TDIFFRAND(prevMs,durationMs) (millis() - prevMs > (durationMs + random(0,10000)))
#define TDIFFSEC(prevMs,durationSec) (millis() - prevMs > (durationSec * 1000))
#define TDIFFMIN(prevMs,durationMin) (millis() - prevMs > (durationMin * 60 * 1000))

// Function declarations
void sendFDRS();
void loadFDRS(float d, uint8_t t, uint16_t id);
void beginFDRS();
void loopFDRS();
void handleCommands();
void handleActions();
void sendMQTT();  // Add this declaration
void handleTime(); // Add time handling function

// Time-related function implementations
bool setTime(time_t t) {
  if(VALID_TS(t)) {
    now = t;
    return true;
  }
  else {
    return false;
  }
}

void handleTime() {
  // Check if we need to reset the time source if not heard from for 2 hours
  if(timeSource.tmNetIf < TMIF_LOCAL && TDIFFMIN(timeSource.tmLastTimeSet,120)) { // Reset time source to default if not heard anything for two hours
    timeSource.tmNetIf = TMIF_NONE;
    timeSource.tmAddress = 0x0000;
    timeSource.tmLastTimeSet = millis();
    timeSource.tmSource = TMS_NONE;
  }
  
  // Send out time to other devices if we have exceeded the time send interval
  if(validTimeFlag && (TIME_SEND_INTERVAL != 0) && TDIFFMIN(lastTimeSend,TIME_SEND_INTERVAL)) {
    lastTimeSend = millis();
    // sendTime(); // Not implemented for this simplified version
  }
}

// Implementation of core FDRS functions

void sendMQTT()
{
  DBG("Sending MQTT.");
  DynamicJsonDocument doc(2048);
  for (int i = 0; i < ln; i++)
  {
    doc[i]["id"] = theData[i].id;
    doc[i]["type"] = theData[i].t;
    doc[i]["data"] = theData[i].d;
    // 在循环中添加yield，防止大数据量时阻塞
    if(i % 10 == 0) yield(); // 每处理10个项目yield一次
  }
  String outgoingString;
  serializeJson(doc, outgoingString);
  
  // 添加超时机制以防止publish阻塞
  unsigned long publishStart = millis();
  const unsigned long MAX_PUBLISH_TIME = 5000; // 5秒超时
  
  if (client.publish(TOPIC_DATA, outgoingString.c_str()))
  {
    DBG("MQTT publish successful");
  }
  else
  {
    // 检查是否超时
    if(millis() - publishStart > MAX_PUBLISH_TIME) {
      DBG("MQTT publish timeout");
      // 重置连接以避免进一步问题
      client.disconnect();
    } else {
      DBG("MQTT publish failed");
    }
  }
  
  // Add yield to prevent watchdog timeout
  yield();
}

void sendFDRS()
{
  if (data_count > 0)
  {
    for (int i = 0; i < data_count; i++)
    {
      theData[i].id = fdrsData[i].id;
      theData[i].t = fdrsData[i].t;
      theData[i].d = fdrsData[i].d;
    }
    ln = data_count;
    data_count = 0;
    newData = event_internal;
    DBG("Entered internal data.");
  }
}

void loadFDRS(float d, uint8_t t, uint16_t id)
{
  // guard against buffer overflow
  if (data_count > 253)
  {
    sendFDRS();
  }
  DBG("Id: " + String(id) + " - Type: " + String(t) + " - Data loaded: " + String(d));
  DataReading dr;
  dr.id = id;
  dr.t = t;
  dr.d = d;
  fdrsData[data_count] = dr;
  data_count++;
}

void beginFDRS()
{
  // Initialize WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  DBG("\n");
  DBG("Initializing FDRS Gateway!");
  DBG("Address: " + String(UNIT_MAC, HEX));
  DBG("Debugging verbosity level: " + String(DBG_LEVEL));
  
  // Initialize timeSource
  timeSource.tmNetIf = TMIF_NONE;
  timeSource.tmAddress = 0xFFFF;
  timeSource.tmSource = TMS_NONE;
  timeSource.tmLastTimeSet = 0;
  
  // Initialize MQTT
  client.setServer(MQTT_ADDR, MQTT_PORT);
  client.setCallback([](char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
  });
}

void handleCommands()
{
  // Handle commands if needed
  theCmd.cmd = cmd_clear;
  theCmd.param = 0;
}

void handleActions()
{
  if (newData != event_clear)
  {
    switch (newData)
    {
    case event_serial:
#ifdef SERIAL_ACT
      SERIAL_ACT  // This will execute sendMQTT() based on config
#endif
      break;
    case event_internal:
#ifdef INTERNAL_ACT
      INTERNAL_ACT  // This will execute sendMQTT() based on config
#endif
      break;
    }
    newData = event_clear;
  }
  
  // Add yield to prevent watchdog timeout
  yield();
}

unsigned long lastReconnectAttempt = 0;
unsigned long lastClientLoop = 0;
const unsigned long CLIENT_LOOP_INTERVAL = 10; // 每10ms调用一次client.loop()

void reconnectMQTT() {
  if (!client.connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      Serial.print("Attempting MQTT connection...");
      if (client.connect("FDRS_UART_Gateway", MQTT_USER, MQTT_PASS)) {
        Serial.println("connected");
        client.subscribe(TOPIC_COMMAND);
        lastClientLoop = now; // 重置client.loop()调用计时器
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println();
      }
    }
  }
}

void loopFDRS()
{
  unsigned long now = millis();
  
  // Handle MQTT connection
  reconnectMQTT();
  
  // Call client.loop() periodically but not every cycle to prevent blocking
  if(now - lastClientLoop > CLIENT_LOOP_INTERVAL) {
    lastClientLoop = now;
    // Add yield before and after to prevent watchdog timeout
    yield();
    client.loop();
    yield();
  }

  // Handle time functions
  handleTime();
  
  // Add yield to prevent watchdog timeout
  yield();

  // Handle commands
  handleCommands();
  
  // Add yield to prevent watchdog timeout
  yield();

  // Handle serial data from PortC (Serial2)
  handleSerialCustom();
  
  // Add yield to prevent watchdog timeout
  yield();

  // Handle actions based on received data
  handleActions();
  
  // Final yield to prevent watchdog timeout
  yield();
}

#endif // FDRS_CUSTOM_GATEWAY_H