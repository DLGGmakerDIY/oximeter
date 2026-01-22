#include "MAX30102_sensor_functions.h"

// 定义全局变量
MAX30105 Sensor;

uint32_t irBuffer[bufferLength];
uint32_t redBuffer[bufferLength];

int32_t spo2, heartRate;
int8_t validSPO2, validHeartRate, old_spo2;

// const byte RATE_SIZE = 5;
uint16_t rate_begin = 0;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
float beatsPerMinute;
int beatAvg;
int8_t flag_Reset = 0;
byte num_fail = 0;

uint32_t red_pos = 0, ir_pos = 0;

uint32_t t1, t2, last_beat = 0, Program_freq = 0;

bool initMAX30102Sensor()
{
    // Serial.begin(115200); // initialize serial communication

    // pinMode(25, INPUT_PULLUP); // set pin mode
    // pinMode(26, OUTPUT);       // 必须添加这行，否则初始化不通过

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN); // initialize I2C - M5StickC Plus固定使用GPIO 0和GPIO 26

    // initialize Sensor
    if (!Sensor.begin(Wire, I2C_SPEED_FAST))
    {
        // init fail
        Serial.println(F("MAX30102 was not found. Please check wiring/power."));
        return false;
    }
    Serial.println(F("Place your index finger on the Sensor with steady pressure"));

    // set Max30102
    Sensor.setup();
    Sensor.setPulseAmplitudeRed(0x0A); // 设置红色LED电流
    Sensor.setPulseAmplitudeIR(0x0A);  // 设置红外LED电流
    // Sensor.clearFIFO();

    return true;
}

void collectSensorData()
{
    // 首先收集100个样本用于SPO2计算
    // 读取前100个样本，并确定信号范围
    for (uint8_t i = 0; i < bufferLength; i++)
    {
        while (Sensor.available() == false) // 是否有新数据？
            Sensor.check();                 // 检查传感器是否有新数据

        redBuffer[i] = Sensor.getRed();
        irBuffer[i] = Sensor.getIR();
        Sensor.nextSample(); // 完成此样本，移动到下一个样本
    }
}

void processSensorData()
{
    // 在前100个样本后计算心率和SpO2
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
}

void getHeartRateAndSPO2(int32_t *hr, int8_t *validHr, int32_t *sp, int8_t *validSp)
{
    *hr = heartRate;
    *validHr = validHeartRate;
    *sp = spo2;
    *validSp = validSPO2;
}

int getAverageHeartRate()
{
    return beatAvg;
}

int getCurrentSPO2()
{
    return spo2;
}

void sensorLoop()
{
    uint16_t ir, red;

    if (flag_Reset)
    {
        Sensor.clearFIFO();
        delay(5);
        flag_Reset = 0;
    }

    while (flag_Reset == 0)
    {
        while (Sensor.available() == false)
        {
            delay(10);
            Sensor.check();
        }

        while (1)
        {
            red = Sensor.getRed();
            ir = Sensor.getIR();

            if ((ir > 1000) && (red > 1000))
            {
                num_fail = 0;
                t1 = millis();
                redBuffer[(red_pos + bufferLength) % bufferLength] = red;
                irBuffer[(ir_pos + bufferLength) % bufferLength] = ir;
                // 检查是否收集了足够的数据进行SPO2计算
                if (red_pos > bufferLength)
                {
                    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength,
                                                           redBuffer, &spo2, &validSPO2,
                                                           &heartRate, &validHeartRate);
                }

                // 计算心率
                t2 = millis();
                Program_freq++;
                if (checkForBeat(ir) == true)
                {
                    long delta = 0;
                    if (last_beat != 0)
                    {
                        delta = millis() - last_beat - (t2 - t1) * (Program_freq - 1);
                    }
                    else
                    {
                        // 首次检测到心跳，不计算BPM，仅记录时间
                        last_beat = millis();
                        Program_freq = 0;
                        continue; // 跳过此次循环，等待下一次心跳
                    }

                    last_beat = millis();

                    Program_freq = 0;
                    if (delta > 0)
                    {
                        beatsPerMinute = 60 / (delta / 1000.0);
                    }
                    else
                    {
                        // 防止负数或零除错误
                        beatsPerMinute = 0;
                    }
                    if ((beatsPerMinute > 30) && (beatsPerMinute < 120))
                    {
                        rate_begin++;
                        if ((abs(beatsPerMinute - beatAvg) > 15) &&
                            ((beatsPerMinute < 55) || (beatsPerMinute > 95)))
                            beatsPerMinute = beatAvg * 0.9 + beatsPerMinute * 0.1;
                        if ((abs(beatsPerMinute - beatAvg) > 10) &&
                            (beatAvg > 60) &&
                            ((beatsPerMinute < 65) || (beatsPerMinute > 90)))
                            beatsPerMinute = beatsPerMinute * 0.4 + beatAvg * 0.6;

                        rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
                        rateSpot %= RATE_SIZE;                    // Wrap variable

                        // Take average of readings
                        beatAvg = 0;
                        if ((beatsPerMinute == 0) && (heartRate > 60) &&
                            (heartRate < 90))
                            beatsPerMinute = heartRate;
                        if (rate_begin >= RATE_SIZE)
                        {
                            for (byte x = 0; x < RATE_SIZE; x++)
                                beatAvg += rates[x];
                            beatAvg /= RATE_SIZE;
                        }
                        else if (rate_begin > 0)
                        {
                            for (byte x = 0; x < rate_begin; x++)
                                beatAvg += rates[x];
                            beatAvg /= rate_begin;
                        }
                        else
                        {
                            beatAvg = (int)beatsPerMinute; // Use current BPM when no history
                        }
                    }
                }
            }

            else
                num_fail++;

            red_pos++;
            ir_pos++;

            if ((Sensor.check() == false) || flag_Reset)
                break;
        }

        old_spo2 = spo2;
        if (!validSPO2)
            spo2 = old_spo2;

        break; // 退出while循环，允许主循环继续
    }
}