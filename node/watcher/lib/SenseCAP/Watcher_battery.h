#ifndef WATCHER_BATTERY_H
#define WATCHER_BATTERY_H

#include <Arduino.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

// 电池ADC配置
#define BSP_BAT_ADC_CHAN (ADC_CHANNEL_2)     // GPIO3
#define BSP_BAT_ADC_ATTEN (ADC_ATTEN_DB_2_5) // 0 ~ 1100 mV
#define BSP_BAT_VOL_RATIO (82.0 / 20.0)      // 分压比

/**
 * @brief SenseCAP Watcher 电池管理类
 * 
 * 用于检测电池电压和计算电池百分比
 */
class Watcher_battery
{
private:
    adc_oneshot_unit_handle_t adc_handle;
    adc_cali_handle_t cali_handle;
    bool adc_initialized;

public:
    /**
     * @brief 构造函数
     */
    Watcher_battery();
    
    /**
     * @brief 初始化电池检测功能
     * @return true 成功, false 失败
     */
    bool beginBattery();
    
    /**
     * @brief 获取电池电压（mV）
     * @return 电池电压值（mV）
     */
    uint16_t getBatteryVoltage();
    
    /**
     * @brief 获取电池百分比
     * @return 电池百分比（0-100）
     */
    uint8_t getBatteryPercent();
};

// 创建全局实例
extern Watcher_battery watcherBattery;

#endif // WATCHER_BATTERY_H