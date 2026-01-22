#ifndef MAX30102_SENSOR_FUNCTIONS_H
#define MAX30102_SENSOR_FUNCTIONS_H

#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

// watcher I2C引脚定义
#define I2C_SDA_PIN 47
#define I2C_SCL_PIN 48

// 传感器配置参数
#define MAX_BRIGHTNESS 255
#define bufferLength 100

// 传感器对象
extern MAX30105 Sensor;

// 数据缓冲区
extern uint32_t irBuffer[bufferLength];
extern uint32_t redBuffer[bufferLength];

// 测量结果变量
extern int32_t spo2, heartRate;
extern int8_t validSPO2, validHeartRate, old_spo2;

// 心率平均值相关
#define RATE_SIZE 5
extern byte rates[RATE_SIZE];
extern byte rateSpot;
extern long lastBeat;
extern float beatsPerMinute;
extern int beatAvg;

// 其他全局变量
extern int8_t flag_Reset;
extern uint16_t rate_begin;
extern byte num_fail;
extern uint32_t red_pos, ir_pos;
extern uint32_t t1, t2, last_beat, Program_freq;

// 初始化函数
bool initMAX30102Sensor();

// 传感器数据采集函数
void collectSensorData();

// 传感器数据处理函数
void processSensorData();

// 获取心率和血氧值的函数
void getHeartRateAndSPO2(int32_t *heartRate, int8_t *validHeartRate, int32_t *spo2, int8_t *validSPO2);

// 获取平均心率的函数
int getAverageHeartRate();

// 获取当前血氧值的函数
int getCurrentSPO2();

// 传感器循环处理函数
void sensorLoop();

#endif // MAX30102_SENSOR_FUNCTIONS_H