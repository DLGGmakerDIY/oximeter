#include "Watcher_battery.h"
#include <Arduino.h>

// 创建全局实例
Watcher_battery watcherBattery;

Watcher_battery::Watcher_battery()
{
    // 初始化成员变量
    adc_handle = NULL;
    cali_handle = NULL;
    adc_initialized = false;
}

bool Watcher_battery::beginBattery()
{
    if (adc_initialized)
    {
        return true;
    }

    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config, &adc_handle);

    adc_oneshot_chan_cfg_t ch_config = {
        .atten = BSP_BAT_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    adc_oneshot_config_channel(adc_handle, BSP_BAT_ADC_CHAN, &ch_config);

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = BSP_BAT_ADC_CHAN,
        .atten = BSP_BAT_ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    if (adc_cali_create_scheme_curve_fitting(&cali_config, &cali_handle) == ESP_OK)
    {
        adc_initialized = true;
        // Serial.println("ADC初始化成功");
    }
    else
    {
        // Serial.println("ADC初始化失败");
    }
    
    return adc_initialized;
}

uint16_t Watcher_battery::getBatteryVoltage()
{
    if (!adc_initialized)
    {
        beginBattery();
    }
    if (adc_initialized)
    {
        int raw_value = 0;
        int voltage = 0; // mV
        adc_oneshot_read(adc_handle, BSP_BAT_ADC_CHAN, &raw_value);
        adc_cali_raw_to_voltage(cali_handle, raw_value, &voltage);
        voltage = voltage * BSP_BAT_VOL_RATIO;
        return (uint16_t)voltage;
    }
    return 0;
}

uint8_t Watcher_battery::getBatteryPercent()
{
    int32_t voltage = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        voltage += getBatteryVoltage();
    }
    voltage /= 10;
    // 简单的电压到百分比转换公式
    int percent = (-1 * voltage * voltage + 9016 * voltage - 19189000) / 10000;
    percent = (percent > 100) ? 100 : (percent < 0) ? 0
                                                    : percent;
    return (uint8_t)percent;
}