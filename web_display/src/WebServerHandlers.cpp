#include "WebServerHandlers.h"

// 全局变量定义
WebServer *globalServer = nullptr;

// WiFi配置（需要根据实际情况修改）
const char *ssid = "DLGG_HOME";
const char *password = "li89400838";

// 监测开始时间
unsigned long monitoringStartTime = 0;

// 初始化全局WebServer实例
void initWebServerInstance(WebServer *server)
{
    globalServer = server;
    monitoringStartTime = millis(); // 记录监测开始时间
}

// 处理根路径请求
void handleRoot()
{
    // 尝试从SPIFFS打开HTML文件并发送
    if (SPIFFS.exists("/index.html"))
    {
        File file = SPIFFS.open("/index.html", "r");
        if (file)
        {
            String htmlContent = file.readString();
            globalServer->send(200, "text/html", htmlContent);
            file.close();
        }
        else
        {
            // 如果文件打开失败，返回错误信息
            globalServer->send(500, "text/plain", "Failed to open file");
        }
    }
    else
    {
        // 如果文件不存在，返回简单的HTML页面
        String html = "<html><body><h1>Oximeter Web Server</h1><p>Device 1 Data:</p>";
        html += "<div id='device1Data'></div>";
        html += "<script>";
        html += "setInterval(function(){";
        html += "  fetch('/getDevice1Data')";
        html += "    .then(response => response.json())";
        html += "    .then(data => {";
        html += "      document.getElementById('device1Data').innerHTML = ";
        html += "        'Heart Rate: ' + (data.hr >= 0 ? data.hr : '--') + '<br>' +";
        html += "        'Oxygen: ' + (data.o2 >= 0 ? data.o2 : '--') + '<br>' +";
        html += "        'Active: ' + data.active + '<br>' +";
        html += "        'Timestamp: ' + data.millis + '<br>' +";
        html += "        'Monitoring Time: ' + data.monitoringTime + 's';";
        html += "    });";
        html += "}, 1000);";
        html += "</script>";
        html += "</body></html>";
        globalServer->send(200, "text/html", html);
    }
}

// 连接WiFi
bool connectToWiFi(const char *ssid, const char *password)
{
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);

    Serial.printf("Connecting to WiFi: %s\n", ssid);
    WiFi.begin(ssid, password);

    int attempts = 0;
    const int maxAttempts = 20; // 最多尝试20次，每次500ms，总共约10秒

    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi connected!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
        return true;
    }
    else
    {
        Serial.println("\nWiFi connection failed!");
        return false;
    }
}

// 设置Web服务器路由
void setupWebServerRoutes()
{
    globalServer->on("/", handleRoot);                            // 注册根路径与对应回调函数
    globalServer->on("/getDevice1Data", handle_getDevice1Data);   // 添加获取设备1数据的路由
    globalServer->on("/getDevice2Data", handle_getDevice2Data);   // 添加获取设备2数据的路由
    globalServer->on("/getBPM_SpO2", handle_getBPM_SpO2);         // 添加获取BPM和SpO2数据的路由（兼容参考程序）
    globalServer->on("/getBPM_SpO22", handle_getBPM_SpO22);       // 添加获取设备2的BPM和SpO2数据的路由（兼容monitor2.js）
    globalServer->on("/getSystemStatus", handle_getSystemStatus); // 添加获取系统状态的路由
    globalServer->onNotFound(handleStaticFile);                   // 处理所有其他静态文件请求
}

// 处理获取设备1的心率和血氧数据的请求
void handle_getDevice1Data()
{
    char raw_JSON[1024];
    DynamicJsonDocument doc(1024);

    // 计算已监测时间（秒）
    unsigned long monitoringTime = (millis() - monitoringStartTime) / 1000;

    // 获取设备1的数据（devices[0]对应ID为1的设备）
    doc["millis"] = millis();
    doc["hr"] = devices[0].active ? devices[0].hr : -1;
    doc["o2"] = devices[0].active ? devices[0].o2 : -1;
    doc["active"] = devices[0].active;
    doc["monitoringTime"] = monitoringTime;

    serializeJson(doc, raw_JSON);
    globalServer->send(200, "application/json", raw_JSON);
}

// 处理获取设备2的心率和血氧数据的请求
void handle_getDevice2Data()
{
    char raw_JSON[1024];
    DynamicJsonDocument doc(1024);

    // 计算已监测时间（秒）
    unsigned long monitoringTime = (millis() - monitoringStartTime) / 1000;

    // 获取设备2的数据（devices[1]对应ID为2的设备）
    doc["millis"] = millis();
    doc["hr"] = devices[1].active ? devices[1].hr : -1;
    doc["o2"] = devices[1].active ? devices[1].o2 : -1;
    doc["active"] = devices[1].active;
    doc["monitoringTime"] = monitoringTime;

    serializeJson(doc, raw_JSON);
    globalServer->send(200, "application/json", raw_JSON);
}

// 处理获取BPM和SpO2数据的请求（兼容参考程序）
void handle_getBPM_SpO2()
{
    char raw_JSON[1024];
    DynamicJsonDocument doc(1024);

    // 计算已监测时间（秒）
    unsigned long monitoringTime = (millis() - monitoringStartTime) / 1000;

    // 获取设备1的数据（devices[0]对应ID为1的设备）
    doc["millis"] = millis();
    doc["BPM"] = devices[0].active ? devices[0].hr : -1;  // 兼容参考程序的BPM字段名
    doc["SpO2"] = devices[0].active ? devices[0].o2 : -1; // 兼容参考程序的SpO2字段名
    doc["monitoringTime"] = monitoringTime;

    serializeJson(doc, raw_JSON);
    globalServer->send(200, "application/json", raw_JSON);
}

// 处理获取设备2的BPM和SpO2数据的请求（兼容monitor2.js）
void handle_getBPM_SpO22()
{
    char raw_JSON[1024];
    DynamicJsonDocument doc(1024);

    // 计算已监测时间（秒）
    unsigned long monitoringTime = (millis() - monitoringStartTime) / 1000;

    // 获取设备2的数据（devices[1]对应ID为2的设备）
    doc["millis"] = millis();
    doc["BPM"] = devices[1].active ? devices[1].hr : -1;  // 兼容monitor2.js的BPM字段名
    doc["SpO2"] = devices[1].active ? devices[1].o2 : -1; // 兼容monitor2.js的SpO2字段名
    doc["monitoringTime"] = monitoringTime;

    serializeJson(doc, raw_JSON);
    globalServer->send(200, "application/json", raw_JSON);
}

// 处理获取系统状态的请求
void handle_getSystemStatus()
{
    char raw_JSON[1024];
    DynamicJsonDocument doc(1024);

    doc["millis"] = millis();
    doc["deviceName"] = "ESP32 Oximeter Web Server";
    doc["STA_IP"] = WiFi.localIP().toString();

    char compilationDate[50];
    sprintf(compilationDate, "%s %s", __DATE__, __TIME__);
    doc["compilationDate"] = compilationDate;

    doc["chipModel"] = ESP.getChipModel();
    doc["chipRevision"] = ESP.getChipRevision();
    doc["cpuFreqMHz"] = ESP.getCpuFreqMHz();
    doc["chipCores"] = ESP.getChipCores();

    doc["heapSizeKiB"] = ESP.getHeapSize() / 1024;
    doc["freeHeapKiB"] = ESP.getFreeHeap() / 1024;

    doc["psramSizeKiB"] = ESP.getPsramSize() / 1024;
    doc["freePsramKiB"] = ESP.getFreePsram() / 1024;

    doc["flashSpeedMHz"] = ESP.getFlashChipSpeed() / 1000000;
    doc["flashSizeMib"] = ESP.getFlashChipSize() / 1024 / 1024;

    doc["sketchMD5"] = ESP.getSketchMD5();
    doc["sdkVersion"] = ESP.getSdkVersion();

    serializeJson(doc, raw_JSON);
    globalServer->send(200, "application/json", raw_JSON);
}