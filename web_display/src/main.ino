//
#include <Arduino.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>   //引入WebServer库
#include <ArduinoJson.h> // 添加ArduinoJson库支持
#include "Free_Fonts.h"  //  M5.Display.setFreeFont(FSS24);
#include "fdrs_gateway_config.h"
// Use our custom FDRS implementation that supports Serial2 (PortC)
#include "fdrs_custom_gateway.h"
#include "OximeterDisplay.h"
#include "SerialDataProcessor.h" // 包含新创建的串口数据处理头文件
#include "WebServerHandlers.h"   // 引入Web服务器处理函数

// 创建全局数组，用于存储多个设备的数据
DeviceData devices[MAX_DEVICES] = {
    {1, 0, 0, false}, // 设备1
    {2, 0, 0, false}  // 设备2
};

// 声明WebServer对象
WebServer server(80);

void setup()
{
    Serial.begin(115200);
    auto cfg = M5.config();
    M5.begin(cfg);

    // 初始化Web服务器
    M5.Display.println("Initializing Web Server...");
    Serial.println("Initializing Web Server...");

    // 初始化全局WebServer实例
    initWebServerInstance(&server);

    // 初始化SPIFFS并检查文件
    if (!initAndCheckSPIFFS())
    {
        M5.Display.println("SPIFFS Mount Failed");
        Serial.println("SPIFFS Mount Failed");
    }

    // 连接WiFi
    if (connectToWiFi(ssid, password))
    {
        M5.Display.print("IP:");
        Serial.print("IP:");
        M5.Display.println(WiFi.localIP());
        Serial.println(WiFi.localIP());

        // 设置Web服务器路由
        setupWebServerRoutes();

        server.begin(); // 启动服务器
        M5.Display.println("Web server started");
        Serial.println("Web server started");
    }
    else
    {
        M5.Display.println("WiFi connection failed!");
        Serial.println("WiFi connection failed!");
    }

    // // 不能放在M5.begin(cfg);之后？无法下载程序，需要G0接GND才能下载
    Serial.println("Waiting for button A press...");
    M5.Display.println("Waiting for button A press...");
    // 等待按键A按下
    while (true)
    {
        M5.update(); // 更新按键状态
        if (M5.BtnA.wasPressed())
        {
            break;
        }
        delay(100); // 每100ms检查一次
    }
    Serial.println("Button A pressed, continuing initialization...");
    M5.Display.println("Button A pressed, continuing initialization...");
    Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // RXD2=13，TXD2=14

    initOximeterDisplay();
}

void loop()
{
    processSerialData();
    checkDeviceTimeout();  // 检查设备数据超时
    server.handleClient(); // 处理Web服务器客户端请求

    if (M5.BtnA.wasPressed())
    {
        // M5.Display.println('A');
        Serial.println('A');
    }
    // 不需要按键和随机更新，只显示固定值
    M5.update();
    delay(10);
}