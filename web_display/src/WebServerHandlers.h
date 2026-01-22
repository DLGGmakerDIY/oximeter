#ifndef WEBSERVER_HANDLERS_H
#define WEBSERVER_HANDLERS_H

/**
 * Web服务器处理函数封装
 * 包含根路径处理、WiFi连接和数据处理功能
 */

#include <WiFi.h>
#include <WebServer.h>
#include <M5Unified.h>
#include <ArduinoJson.h>
#include "sysSPIFFS.h"           // 引入SPIFFS相关功能
#include "SerialDataProcessor.h" // 引入设备数据结构

// 声明全局WebServer实例
extern WebServer *globalServer;

// WiFi配置参数
extern const char *ssid;
extern const char *password;

// 顺序数相关变量
extern unsigned long sequenceNumber;

// 初始化全局WebServer实例
void initWebServerInstance(WebServer *server);

// 处理根路径请求
void handleRoot();

// 连接WiFi
bool connectToWiFi(const char *ssid, const char *password);

// 设置Web服务器路由
void setupWebServerRoutes();

// 处理获取设备1的心率和血氧数据的请求
void handle_getDevice1Data();

// 处理获取设备2的心率和血氧数据的请求
void handle_getDevice2Data();

// 处理获取BPM和SpO2数据的请求（兼容参考程序）
void handle_getBPM_SpO2();

// 处理获取设备2的BPM和SpO2数据的请求（兼容monitor2.js）
void handle_getBPM_SpO22();

// 处理获取系统状态的请求
void handle_getSystemStatus();

#endif // WEBSERVER_HANDLERS_H