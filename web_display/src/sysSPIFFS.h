#ifndef SYSSPIFFS_H
#define SYSSPIFFS_H

/**
 * SPIFFS系统功能封装
 * 包含SPIFFS初始化、文件存在性检查和静态文件处理功能
 */

#include <SPIFFS.h>
#include <WebServer.h>
#include <M5Unified.h>
#include <ArduinoJson.h>

// 声明全局WebServer实例（用于静态文件处理）
extern WebServer *globalServer;

// 初始化SPIFFS
bool initSPIFFS();

// 检查文件是否存在
bool checkFileExists(const char *filePath);

// 检查多个文件是否存在
void checkFilesExist(const char **filePaths, int count);

// 处理静态文件请求
void handleStaticFile();

// 初始化SPIFFS并检查文件
bool initAndCheckSPIFFS();

#endif // SYSSPIFFS_H