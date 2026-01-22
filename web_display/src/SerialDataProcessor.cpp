#include "SerialDataProcessor.h"
#include "OximeterDisplay.h"
#include "fdrs_gateway_config.h"

#define DATA_TIMEOUT 3000 // 数据超时时间（毫秒）

// 检查设备数据是否超时
void checkDeviceTimeout()
{
    unsigned long currentTime = millis();

    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (devices[i].active)
        {
            // 检查是否超时
            if (currentTime - devices[i].lastUpdateTime > DATA_TIMEOUT)
            {
                devices[i].active = false; // 标记为未激活
#ifdef FDRS_DEBUG
                Serial.printf("设备 %d 数据超时，已标记为未激活\n", devices[i].id);
#endif

                // 更新显示为 "--"
                updateOximeterDisplay(i, 0, 0, false);
            }
        }
    }
}

// 实现processSerialData函数
void processSerialData()
{
    if (Serial2.available())
    {
        String serialData = Serial2.readStringUntil('\n');
        serialData.trim(); // Remove any trailing whitespace
#ifdef FDRS_DEBUG
        Serial.println("Received: " + serialData);
#endif

        // 解析JSON字符串
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, serialData);

        if (error)
        {
            Serial.print(F("JSON解析错误: "));
            Serial.println(error.f_str());
            return;
        }

        // 检查JSON是否为数组格式
        if (!doc.is<JsonArray>())
        {
            Serial.println(F("JSON格式错误: 不是数组格式"));
            return;
        }

        // 遍历JSON数组中的每个对象
        JsonArray array = doc.as<JsonArray>();
        for (JsonObject obj : array)
        {
            int id = obj["id"];
            int type = obj["type"];
            int data = obj["data"];

            // 根据id找到对应的设备
            for (int i = 0; i < MAX_DEVICES; i++)
            {
                if (devices[i].id == id)
                {
                    // 更新设备数据
                    if (type == 11)
                    { // 心率
                        devices[i].hr = data;
                    }
                    else if (type == 10)
                    { // 血氧
                        devices[i].o2 = data;
                    }
                    devices[i].active = true;             // 标记设备为激活
                    devices[i].lastUpdateTime = millis(); // 记录更新时间
                    break;
                }
            }
        }

// 打印解析后的数据，用于调试
#ifdef FDRS_DEBUG
        for (int i = 0; i < MAX_DEVICES; i++)
        {
            if (devices[i].active)
            {
                Serial.print("设备 ");
                Serial.print(devices[i].id);
                Serial.print(": HR=");
                Serial.print(devices[i].hr);
                Serial.print(", O2=");
                Serial.println(devices[i].o2);
            }
        }
#endif

        // 更新界面显示 - 目前只显示前两个设备的数据
        updateOximeterDisplay(0, devices[0].hr, devices[0].o2, devices[0].active);
        updateOximeterDisplay(1, devices[1].hr, devices[1].o2, devices[1].active);
    }
}