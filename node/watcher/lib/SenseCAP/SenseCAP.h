#pragma once

#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "Watcher_battery.h"
#include "Watcher_audio.h"
#include "Watcher_battery_icon.h"

// I2C0
#define I2C0_SDA_PIN 47
#define I2C0_SCL_PIN 48

/* RGB LED */
#define BSP_RGB_CTRL (GPIO_NUM_40)

/* Knob */
#define BSP_KNOB_A (GPIO_NUM_41)
#define BSP_KNOB_B (GPIO_NUM_42)
#define BSP_KNOB_BTN (IO_EXPANDER_PIN_NUM_3)

// IO引脚位掩码定义 (16位，用于PCA9535)
#define IO_PIN_0 (1 << 0)
#define IO_PIN_1 (1 << 1)
#define IO_PIN_2 (1 << 2)
#define IO_PIN_3 (1 << 3)
#define IO_PIN_4 (1 << 4)
#define IO_PIN_5 (1 << 5)
#define IO_PIN_6 (1 << 6)
#define IO_PIN_7 (1 << 7)
#define IO_PIN_8 (1 << 8)
#define IO_PIN_9 (1 << 9)
#define IO_PIN_10 (1 << 10)
#define IO_PIN_11 (1 << 11)
#define IO_PIN_12 (1 << 12)
#define IO_PIN_13 (1 << 13)
#define IO_PIN_14 (1 << 14)
#define IO_PIN_15 (1 << 15)

// 背光引脚配置
#define LCD_BACKLIGHT_PIN 8 // 背光控制引脚

/**
 * @brief SenseCAP 电源管理库
 *
 * 用于控制SenseCAP Watcher开发板的各种电源输出，
 * 包括Grove接口、LCD电源、RGB LED、滚轮按键等功能。
 */

class SenseCAP
{
public:
    /**
     * @brief 构造函数
     */
    SenseCAP();

    /**
     * @brief 初始化电源管理模块（包括I2C总线和PCA9535 IO扩展器）
     * @return true 成功, false 失败
     */
    bool beginPowerManager();

    /**
     * @brief 初始化完整的系统功能（包括电源管理和滚轮功能）
     * @return true 成功, false 失败
     */
    bool begin();

    /**
     * @brief 初始化电源管理模块并打开Grove电源
     * @return true 成功, false 失败
     */
    bool beginWithGrovePower();

    /**
     * @brief 初始化完整的滚轮功能（RGB LED + 按键 + 旋转编码器）
     * @param rgb_pin RGB LED引脚，默认GPIO 40
     * @param knob_a_pin A相引脚，默认GPIO 41
     * @param knob_b_pin B相引脚，默认GPIO 42
     * @return true 成功, false 失败
     */
    bool beginWheel(uint8_t rgb_pin = BSP_RGB_CTRL, uint8_t knob_a_pin = BSP_KNOB_A, uint8_t knob_b_pin = BSP_KNOB_B);

    /**
     * @brief 控制电源输出状态
     * @param pin_mask 电源控制位掩码 (BSP_PWR_* 宏)
     * @param level 电平状态: 0=关闭, 1=打开
     * @return true 成功, false 失败
     */
    bool setPowerLevel(uint8_t pin_mask, uint8_t level);

    /**
     * @brief 打开Grove电源
     * @return true 成功, false 失败
     */
    bool enableGrovePower();

    /**
     * @brief 关闭Grove电源
     * @return true 成功, false 失败
     */
    bool disableGrovePower();

    /**
     * @brief 打开LCD电源
     * @return true 成功, false 失败
     */
    bool enableLCDPower();

    /**
     * @brief 关闭LCD电源
     * @return true 成功, false 失败
     */
    bool disableLCDPower();

    /**
     * @brief 打开所有系统电源
     * @return true 成功, false 失败
     */
    bool enableAllSystemPower();

    /**
     * @brief 关闭所有电源
     * @return true 成功, false 失败
     */
    bool disableAllPower();

    // ===== 背光控制功能 =====

    /**
     * @brief 初始化背光控制
     * @return true 成功, false 失败
     */
    bool initBacklight();

    /**
     * @brief 设置背光亮度
     * @param brightness 亮度值 (0-1023, 对应PWM占空比)
     */
    void setBacklight(uint16_t brightness);

    /**
     * @brief 获取当前背光亮度
     * @return 当前亮度值 (0-1023)
     */
    uint16_t getBacklight();

    /**
     * @brief 设置背光为最大亮度
     */
    void setBacklightMax();

    /**
     * @brief 设置背光为中等亮度
     */
    void setBacklightMedium();

    /**
     * @brief 设置背光为最小亮度（关闭）
     */
    void setBacklightOff();

    /**
     * @brief 获取当前电源状态
     * @return Port 1的输出状态字节
     */
    uint8_t getPowerStatus();

    /**
     * @brief 获取滚轮按键状态
     * @return true=按下, false=释放
     */
    bool getWheelButtonState();

    // ===== RGB LED 控制功能 =====

    /**
     * @brief 初始化RGB LED
     * @param rgb_pin RGB LED引脚，默认GPIO 40
     * @return true 成功, false 失败
     */
    bool initRGB(uint8_t rgb_pin = 40);

    /**
     * @brief 设置RGB LED颜色
     * @param r 红色分量 (0-255)
     * @param g 绿色分量 (0-255)
     * @param b 蓝色分量 (0-255)
     */
    void setRGB(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief 关闭RGB LED
     */
    void turnOffRGB();

    /**
     * @brief 打开RGB LED（显示当前颜色）
     */
    void turnOnRGB();

    /**
     * @brief 切换RGB LED开关状态
     */
    void toggleRGB();

    /**
     * @brief 改变RGB LED颜色（循环红绿蓝）
     * @param reverse true=逆时针旋转, false=顺时针旋转
     */
    void changeRGBColor(bool reverse);

    /**
     * @brief 获取当前RGB LED颜色
     * @param r 红色分量输出
     * @param g 绿色分量输出
     * @param b 蓝色分量输出
     */
    void getCurrentRGB(uint8_t &r, uint8_t &g, uint8_t &b);

    /**
     * @brief 获取RGB LED开关状态
     * @return true=开启, false=关闭
     */
    bool isRGBOn();

    /**
     * @brief 将角度转换为RGB颜色（彩虹色）
     * @param angle 角度值 (0-360度)
     * @param r 红色分量输出
     * @param g 绿色分量输出
     * @param b 蓝色分量输出
     */
    void angleToRGB(float angle, uint8_t &r, uint8_t &g, uint8_t &b);

    // ===== 滚轮按键控制功能 =====

    /**
     * @brief 初始化滚轮按键
     * @param knob_a_pin A相引脚，默认GPIO 41
     * @param knob_b_pin B相引脚，默认GPIO 42
     * @return true 成功, false 失败
     */
    bool initWheelButton(uint8_t knob_a_pin = 41, uint8_t knob_b_pin = 42);

    /**
     * @brief 检查滚轮按键状态（需要在主循环中调用）
     * 处理短按（切换LED）和长按（关机）事件
     */
    void checkWheelButton();

    /**
     * @brief 执行系统关机
     * 检查充电状态，如果在充电只关闭LED，否则真正关机
     */
    void systemShutdown();

    /**
     * @brief 处理滚轮按键用于屏幕控制（需要在主循环中调用）
     * 短按（200ms-2s）: 唤醒屏幕
     * 长按（2s以上）: 关闭屏幕
     */
    void handleWheelButtonForScreen();

    /**
     * @brief 检查是否正在充电
     * @return true=正在充电, false=未充电
     */
    bool isCharging();

    /**
     * @brief 执行工厂重置
     * 清除NVS存储并重启设备
     */
    void factoryReset();

    /**
     * @brief 执行系统关机
     * 检查充电状态，如果不在充电则关闭系统电源
     */
    void systemPowerOff();

    // ===== 旋转编码器功能 =====

    /**
     * @brief 初始化旋转编码器
     * @param knob_a_pin A相引脚，默认GPIO 41
     * @param knob_b_pin B相引脚，默认GPIO 42
     * @return true 成功, false 失败
     */
    bool initRotaryEncoder(uint8_t knob_a_pin = 41, uint8_t knob_b_pin = 42);

    /**
     * @brief 检查旋转编码器状态（需要在主循环中调用）
     * 旋转时会自动改变RGB颜色
     */
    void checkRotaryEncoder();

    /**
     * @brief 获取旋转编码器位置
     * @return 当前位置值
     */
    int getEncoderPosition();

    // ===== 电池管理功能（包装Watcher_battery类）=====

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
    
    // ===== 电池图标功能（包装Watcher_battery_icon类）=====
    
    /**
     * @brief 创建电池图标（使用对齐方式）
     * @param parent 父对象指针（通常是屏幕）
     * @param percentage 电量百分比（0-100）
     * @param align 对齐方式（LV_ALIGN_*）
     * @param x_offset x轴偏移量
     * @param y_offset y轴偏移量
     * @return 创建的电池图标对象指针
     */
    lv_obj_t *create_battery_icon(lv_obj_t *parent, int percentage, lv_align_t align, int x_offset, int y_offset);
    
    /**
     * @brief 创建电池图标（使用绝对位置）
     * @param parent 父对象指针（通常是屏幕）
     * @param percentage 电量百分比（0-100）
     * @param x 屏幕x坐标（左上角为原点）
     * @param y 屏幕y坐标（左上角为原点）
     * @return 创建的电池图标对象指针
     */
    lv_obj_t *create_battery_icon_abs(lv_obj_t *parent, int percentage, int x, int y);
    
    /**
     * @brief 更新电池图标
     * @param battery_icon 电池图标对象指针
     * @param percentage 电量百分比（0-100）
     */
    void update_battery_icon(lv_obj_t *battery_icon, int percentage);
    
    /**
     * @brief 创建电池图标并自动显示当前电池电量（使用对齐方式）
     * @param parent 父对象指针（通常是屏幕）
     * @param align 对齐方式（LV_ALIGN_*）
     * @param x_offset x轴偏移量
     * @param y_offset y轴偏移量
     * @return 创建的电池图标对象指针
     */
    lv_obj_t *create_battery_icon_auto(lv_obj_t *parent, lv_align_t align, int x_offset, int y_offset);
    
    /**
     * @brief 创建电池图标并自动显示当前电池电量（使用绝对位置）
     * @param parent 父对象指针（通常是屏幕）
     * @param x 屏幕x坐标（左上角为原点）
     * @param y 屏幕y坐标（左上角为原点）
     * @return 创建的电池图标对象指针
     */
    lv_obj_t *create_battery_icon_auto_abs(lv_obj_t *parent, int x, int y);

    // ===== 音频控制功能（包装Watcher_audio类）=====

    /**
     * @brief 初始化音频模块
     * @return true 成功, false 失败
     */
    bool initAudio();

    /**
     * @brief 初始化音频模块并播放指定的音频文件
     * @param filename 音频文件名（SPIFFS中的文件）
     * @return true 成功, false 失败
     */
    bool initAudioAndPlay(const char *filename);

    /**
     * @brief 播放SPIFFS中的音频文件
     * @param filename 音频文件名
     * @return true 成功, false 失败
     */
    bool playAudioFile(const char *filename);

    /**
     * @brief 设置音频音量
     * @param volume 音量值 (0-21)
     */
    void setAudioVolume(uint8_t volume);

    /**
     * @brief 设置音频编解码器音量
     * @param volume 音量值 (0-100)
     */
    void setCodecVolume(uint8_t volume);

    /**
     * @brief 使能音频PA放大器
     * @return true 成功, false 失败
     */
    bool enableAudioPA();

    /**
     * @brief 禁用音频PA放大器
     * @return true 成功, false 失败
     */
    bool disableAudioPA();

    /**
     * @brief 音频循环处理
     * 需要在主循环中调用
     */
    void audioLoop();

    /**
     * @brief 检查音频初始化状态
     * @return true 已初始化, false 未初始化
     */
    bool isAudioInitialized();

private:
    // PCA9535 IO扩展器地址和寄存器
    static const uint8_t PCA9535_ADDR = 0x21;
    static const uint8_t PCA9535_CONFIG_PORT0 = 0x06;
    static const uint8_t PCA9535_CONFIG_PORT1 = 0x07;
    static const uint8_t PCA9535_OUTPUT_PORT0 = 0x02;
    static const uint8_t PCA9535_OUTPUT_PORT1 = 0x03;

    // 电源控制位定义 (Port 1)
    static const uint8_t BSP_PWR_SDCARD = 0x01;   // P1.0
    static const uint8_t BSP_PWR_LCD = 0x02;      // P1.1
    static const uint8_t BSP_PWR_SYSTEM = 0x04;   // P1.2
    static const uint8_t BSP_PWR_AI_CHIP = 0x08;  // P1.3
    static const uint8_t BSP_PWR_CODEC_PA = 0x10; // P1.4
    static const uint8_t BSP_PWR_BAT_DET = 0x20;  // P1.5
    static const uint8_t BSP_PWR_GROVE = 0x40;    // P1.6
    static const uint8_t BSP_PWR_BAT_ADC = 0x80;  // P1.7

    // 电池图标相关
    Watcher_battery_icon &watcherBatteryIcon;

    // ===== 私有成员变量 =====

    // RGB LED 相关
    Adafruit_NeoPixel *rgb;
    bool rgbInitialized;
    bool ledOn;
    uint8_t currentColor[3];
    uint8_t targetColor[3];
    uint8_t rgbPin;
    uint8_t colorStep;

    // 背光相关
    uint16_t backlightBrightness;
    bool backlightInitialized;

    // 旋转编码器相关
    volatile int encoderPos;
    volatile int lastEncoded;
    volatile bool encoderChanged;
    uint8_t knobAPin;
    uint8_t knobBPin;
    bool encoderInitialized;

    // 电源管理相关
    bool powerManagerInitialized;

    // 按键相关
    bool btnPressed;
    bool btnLongPressed;
    unsigned long btnPressTime;
    bool btnWasPressed;
    bool buttonInitialized;

    // 屏幕控制按键相关
    bool screenAwake;
    unsigned long screenBtnPressTime;
    bool screenBtnWasPressed;
    bool screenBtnLongPressed;

    // 长按关机相关
    uint32_t long_press_cnt_;

    // 电池相关
    Watcher_battery &watcherBattery;

    // 音频相关
    Watcher_audio &watcherAudio;

    // 按键时间定义
    static const unsigned long SHORT_PRESS_TIME = 200; // 短按时间（ms）
    static const unsigned long LONG_PRESS_TIME = 2000; // 长按时间（ms）
    static const unsigned long DEBOUNCE_TIME = 50;     // 去抖时间（ms）
    static const unsigned long CHECK_INTERVAL = 100;   // 检查间隔（ms）

    // IO扩展器引脚定义
    static const uint16_t EXIO_KNOB_BTN = IO_PIN_3;    // 滚轮按键 (P0.3)
    static const uint16_t EXIO_PWR_SYSTEM = IO_PIN_10; // 系统电源控制 (P1.2)
    static const uint16_t EXIO_PWR_VBUS = IO_PIN_2;    // USB充电检测 (P0.2)

    // 私有成员函数
    bool writeRegister(uint8_t reg, uint8_t data);
    uint8_t readRegister(uint8_t reg);
    bool initPCA9535();

    // RGB LED 私有函数
    void updateLedColor(bool reverse);

    // 旋转编码器私有函数
    void IRAM_ATTR encoderISR();
    void updateEncoder();
    void handleEncoderRGBChange(bool clockwise);
    static void encoderISRStatic(void *arg);

    // 按键私有函数
    void checkButtonInternal();
};

// 为了向后兼容，提供全局函数接口
extern SenseCAP senseCAP;

/**
 * @brief 全局电源控制函数 (向后兼容)
 * @param pin_mask 电源控制位掩码
 * @param level 电平状态: 0=关闭, 1=打开
 * @return true 成功, false 失败
 */
bool bsp_exp_io_set_level(uint8_t pin_mask, uint8_t level);

/**
 * @brief 打开Grove电源 (向后兼容)
 * @return true 成功, false 失败
 */
bool enableGrovePower();

/**
 * @brief 关闭Grove电源 (向后兼容)
 * @return true 成功, false 失败
 */
bool disableGrovePower();

/**
 * @brief 初始化电源管理并打开Grove电源 (向后兼容)
 * @return true 成功, false 失败
 */
bool beginWithGrovePower();
