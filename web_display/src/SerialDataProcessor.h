#ifndef SERIAL_DATA_PROCESSOR_H
#define SERIAL_DATA_PROCESSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>

// 定义最大设备数量
#define MAX_DEVICES 2

// 单个设备的数据结构体
typedef struct
{
    int id;                       // 设备ID
    int hr;                       // 心率数值
    int o2;                       // 血氧数值
    bool active;                  // 设备是否激活
    unsigned long lastUpdateTime; // 最后一次更新的时间戳
} DeviceData;

// 声明全局设备数组，用于在多个文件中共享
extern DeviceData devices[MAX_DEVICES];

// 处理串口接收到的数据，解析JSON并更新界面
void processSerialData();

// 检查设备数据是否超时
void checkDeviceTimeout();

#endif // SERIAL_DATA_PROCESSOR_H