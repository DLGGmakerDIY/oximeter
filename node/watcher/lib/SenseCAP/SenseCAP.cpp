#include "SenseCAP.h"
#include <Arduino.h>
#include <WiFi.h>
#include <nvs_flash.h>

// 创建全局实例
SenseCAP senseCAP;

SenseCAP::SenseCAP() : watcherBattery(::watcherBattery), watcherAudio(::watcherAudio), watcherBatteryIcon(::watcherBatteryIcon)
{
    // RGB LED相关变量初始化
    rgb = nullptr;
    rgbInitialized = false;
    ledOn = false;
    rgbPin = 40;
    colorStep = 16;
    currentColor[0] = 0; // 初始化为黑色
    currentColor[1] = 0;
    currentColor[2] = 0;
    targetColor[0] = 0;
    targetColor[1] = 0;
    targetColor[2] = 0;

    // 旋转编码器相关变量初始化
    encoderPos = 0;
    lastEncoded = 0;
    encoderChanged = false;
    knobAPin = 41;
    knobBPin = 42;
    encoderInitialized = false;

    // 电源管理相关变量初始化
    powerManagerInitialized = false;

    // 按键相关变量初始化
    btnPressed = false;
    btnLongPressed = false;
    btnPressTime = 0;
    btnWasPressed = false;
    buttonInitialized = false;

    // 屏幕控制按键相关变量初始化
    screenAwake = true; // 默认屏幕是唤醒状态
    screenBtnPressTime = 0;
    screenBtnWasPressed = false;
    screenBtnLongPressed = false;

    // 长按关机相关变量初始化
    long_press_cnt_ = 0;

    // 背光相关变量初始化
    backlightBrightness = 512; // 默认中等亮度
    backlightInitialized = false;
}

bool SenseCAP::beginPowerManager()
{
    // 检查是否已经初始化
    if (powerManagerInitialized)
    {
        return true;
    }

    bool result = initPCA9535();
    if (result)
    {
        // 确保RGB灯被初始化并熄灭
        if (!rgbInitialized)
        {
            initRGB(rgbPin);
        }
        turnOffRGB(); // 初始化时熄灭RGB灯

        // 标记电源管理模块已初始化
        powerManagerInitialized = true;
    }
    return result;
}

bool SenseCAP::begin()
{
    // 初始化I2C总线
    if (!Wire.begin(I2C0_SDA_PIN, I2C0_SCL_PIN))
    {
        Serial.println("I2C init failed!");
        return false;
    }
    Wire.setClock(400000);

    // 初始化电源管理模块
    if (!beginPowerManager())
    {
        return false;
    }

    // 初始化滚轮功能（RGB LED + 按键 + 旋转编码器）
    if (!beginWheel())
    {
        return false;
    }

    // 初始化音频模块
    if (!initAudio())
    {
        return false;
    }

    // 初始化电池检测功能
    if (!beginBattery())
    {
        return false;
    }

    return true;
}

bool SenseCAP::beginWithGrovePower()
{
    // 初始化电源管理模块
    if (!beginPowerManager())
    {
        return false;
    }

    // 打开Grove电源
    if (!enableGrovePower())
    {
        return false;
    }

    return true;
}

bool SenseCAP::beginWheel(uint8_t rgb_pin, uint8_t knob_a_pin, uint8_t knob_b_pin)
{
    // 初始化电源管理模块
    // if (!beginPowerManager())
    // {
    //     return false;
    // }

    // 初始化各个功能模块
    if (!initRGB(rgb_pin))
    {
        return false;
    }

    if (!initWheelButton(knob_a_pin, knob_b_pin))
    {
        return false;
    }

    if (!initRotaryEncoder(knob_a_pin, knob_b_pin))
    {
        return false;
    }

    return true;
}

bool SenseCAP::setPowerLevel(uint8_t pin_mask, uint8_t level)
{
    // 读取当前Port 1的输出状态
    uint8_t current_output = readRegister(PCA9535_OUTPUT_PORT1);
    if (current_output == 0xFF)
    {
        return false;
    }

    uint8_t new_output;

    if (level == 1)
    {
        // 设置位为1
        new_output = current_output | pin_mask;
    }
    else
    {
        // 设置位为0
        new_output = current_output & ~pin_mask;
    }

    // 写入新的输出状态
    return writeRegister(PCA9535_OUTPUT_PORT1, new_output);
}

bool SenseCAP::enableGrovePower()
{
    return setPowerLevel(BSP_PWR_GROVE, 1);
}

bool SenseCAP::disableGrovePower()
{
    return setPowerLevel(BSP_PWR_GROVE, 0);
}

bool SenseCAP::enableLCDPower()
{
    return setPowerLevel(BSP_PWR_LCD, 1);
}

bool SenseCAP::disableLCDPower()
{
    return setPowerLevel(BSP_PWR_LCD, 0);
}

bool SenseCAP::enableAllSystemPower()
{
    // 使能系统所需的所有电源：SYSTEM | LCD | SDCARD | AI_CHIP | CODEC_PA | GROVE | BAT_ADC
    // 0x04 | 0x02 | 0x01 | 0x08 | 0x10 | 0x40 | 0x80 = 0xDF
    return writeRegister(PCA9535_OUTPUT_PORT1, 0xDF);
}

bool SenseCAP::disableAllPower()
{
    return writeRegister(PCA9535_OUTPUT_PORT1, 0x00);
}

// ===== 背光控制功能实现 =====
bool SenseCAP::initBacklight()
{
    if (backlightInitialized)
    {
        return true;
    }

    pinMode(LCD_BACKLIGHT_PIN, OUTPUT);
    // 使用analogWrite代替LEDC API
    analogWrite(LCD_BACKLIGHT_PIN, 0); // 初始关闭背光

    backlightInitialized = true;
    return true;
}

void SenseCAP::setBacklight(uint16_t brightness)
{
    if (!backlightInitialized)
    {
        initBacklight();
    }

    if (brightness > 1023)
    {
        brightness = 1023;
    }

    backlightBrightness = brightness;
    analogWrite(LCD_BACKLIGHT_PIN, brightness);
}

uint16_t SenseCAP::getBacklight()
{
    return backlightBrightness;
}

void SenseCAP::setBacklightMax()
{
    setBacklight(1023);
}

void SenseCAP::setBacklightMedium()
{
    setBacklight(512);
}

void SenseCAP::setBacklightOff()
{
    setBacklight(0);
}

uint8_t SenseCAP::getPowerStatus()
{
    return readRegister(PCA9535_OUTPUT_PORT1);
}

bool SenseCAP::getWheelButtonState()
{
    uint8_t port0_state = readRegister(0x00); // 读取输入端口0
    return (port0_state & 0x08) == 0;         // P0.3按键，按下为低电平
}

// ===== PCA9535 IO扩展器功能实现 =====
bool SenseCAP::initPCA9535()
{
    // 配置Port 0为输入模式 (0xFF)
    if (!writeRegister(PCA9535_CONFIG_PORT0, 0xFF))
    {
        return false;
    }

    // 配置Port 1为输出模式 (0x00)
    if (!writeRegister(PCA9535_CONFIG_PORT1, 0x00))
    {
        return false;
    }

    return true;
}

bool SenseCAP::writeRegister(uint8_t reg, uint8_t data)
{
    Wire.beginTransmission(PCA9535_ADDR);
    Wire.write(reg);
    Wire.write(data);
    uint8_t result = Wire.endTransmission();

    return (result == 0);
}

uint8_t SenseCAP::readRegister(uint8_t reg)
{
    Wire.beginTransmission(PCA9535_ADDR);
    Wire.write(reg);
    uint8_t result = Wire.endTransmission();

    if (result != 0)
    {
        return 0xFF; // 错误返回值
    }

    Wire.requestFrom(PCA9535_ADDR, 1);
    if (Wire.available())
    {
        return Wire.read();
    }

    return 0xFF; // 错误返回值
}

// ===== 全局函数实现将在文件末尾提供 =====

// ===== 其他功能实现（简化版本，仅保留核心功能） =====

// RGB LED功能实现（简化版）
bool SenseCAP::initRGB(uint8_t rgb_pin)
{
    if (rgbInitialized)
    {
        return true;
    }

    rgbPin = rgb_pin;
    rgb = new Adafruit_NeoPixel(1, rgbPin, NEO_GRB + NEO_KHZ800);
    if (rgb != nullptr)
    {
        rgb->begin();
        rgb->setPixelColor(0, 0, 0, 0);
        rgb->show();
        rgbInitialized = true;
        return true;
    }

    return false;
}

void SenseCAP::setRGB(uint8_t r, uint8_t g, uint8_t b)
{
    if (!rgbInitialized)
    {
        initRGB(rgbPin);
    }

    if (rgb != nullptr)
    {
        currentColor[0] = r;
        currentColor[1] = g;
        currentColor[2] = b;

        if (ledOn)
        {
            rgb->setPixelColor(0, r, g, b);
            rgb->show();
        }
    }
}

void SenseCAP::turnOffRGB()
{
    if (rgbInitialized && rgb != nullptr)
    {
        rgb->setPixelColor(0, 0, 0, 0);
        rgb->show();
        ledOn = false;
    }
}

void SenseCAP::turnOnRGB()
{
    if (rgbInitialized && rgb != nullptr)
    {
        rgb->setPixelColor(0, currentColor[0], currentColor[1], currentColor[2]);
        rgb->show();
        ledOn = true;
    }
}

// 其他简化功能（旋转编码器、按键等，仅保留接口）
bool SenseCAP::initRotaryEncoder(uint8_t knob_a_pin, uint8_t knob_b_pin)
{
    if (encoderInitialized)
    {
        return true;
    }

    knobAPin = knob_a_pin;
    knobBPin = knob_b_pin;

    pinMode(knobAPin, INPUT_PULLUP);
    pinMode(knobBPin, INPUT_PULLUP);

    encoderInitialized = true;
    return true;
}
int SenseCAP::getEncoderPosition()
{
    return encoderPos;
}

// ===== 电池管理功能实现（包装Watcher_battery类）=====

bool SenseCAP::beginBattery()
{
    return watcherBattery.beginBattery();
}

uint16_t SenseCAP::getBatteryVoltage()
{
    return watcherBattery.getBatteryVoltage();
}

uint8_t SenseCAP::getBatteryPercent()
{
    return watcherBattery.getBatteryPercent();
}

// ===== 电池图标功能实现 =====

lv_obj_t *SenseCAP::create_battery_icon(lv_obj_t *parent, int percentage, lv_align_t align, int x_offset, int y_offset)
{
    return watcherBatteryIcon.create_battery_icon(parent, percentage, align, x_offset, y_offset);
}

lv_obj_t *SenseCAP::create_battery_icon_abs(lv_obj_t *parent, int percentage, int x, int y)
{
    return watcherBatteryIcon.create_battery_icon_abs(parent, percentage, x, y);
}

void SenseCAP::update_battery_icon(lv_obj_t *battery_icon, int percentage)
{
    watcherBatteryIcon.update_battery_icon(battery_icon, percentage);
}

lv_obj_t *SenseCAP::create_battery_icon_auto(lv_obj_t *parent, lv_align_t align, int x_offset, int y_offset)
{
    int percentage = getBatteryPercent();
    return watcherBatteryIcon.create_battery_icon(parent, percentage, align, x_offset, y_offset);
}

lv_obj_t *SenseCAP::create_battery_icon_auto_abs(lv_obj_t *parent, int x, int y)
{
    int percentage = getBatteryPercent();
    return watcherBatteryIcon.create_battery_icon_abs(parent, percentage, x, y);
}

// 其他简化功能
bool SenseCAP::initWheelButton(uint8_t knob_a_pin, uint8_t knob_b_pin)
{
    // 滚轮按键不需要额外初始化
    return true;
}

// 简化的系统功能
bool SenseCAP::isCharging()
{
    // 通过读取P0.2引脚状态检测充电状态
    // P0.2为低电平时表示正在充电
    uint8_t port0_config = readRegister(PCA9535_CONFIG_PORT0);
    return ((port0_config & IO_PIN_2) == 0);
}

void SenseCAP::systemPowerOff()
{
    Serial.println("执行系统关机...");

    // 无论是否充电，都先关闭RGB灯
    turnOffRGB();
    Serial.println("RGB灯已关闭");

    // 检查是否正在充电
    if (isCharging())
    {
        Serial.println("正在充电，仅关闭屏幕背光");
        setBacklightOff();
        return;
    }

    // 不在充电状态，关闭系统电源
    // 设置 BSP_PWR_SYSTEM (P1.2) 为低电平
    uint8_t current_port1 = readRegister(PCA9535_OUTPUT_PORT1);
    uint8_t new_port1 = current_port1 & ~BSP_PWR_SYSTEM; // 清除 BSP_PWR_SYSTEM 位

    Serial.printf("系统电源关闭: 0x%02X -> 0x%02X\n", current_port1, new_port1);
    writeRegister(PCA9535_OUTPUT_PORT1, new_port1);

    // 短暂延迟确保电源控制生效
    delay(100);
}

// 其他简化功能（仅保留接口，不实现完整逻辑）
void SenseCAP::checkWheelButton() {}
void SenseCAP::systemShutdown() {}
void SenseCAP::factoryReset() {}
void SenseCAP::checkRotaryEncoder() {}
void SenseCAP::toggleRGB() {}
void SenseCAP::changeRGBColor(bool reverse) {}
void SenseCAP::getCurrentRGB(uint8_t &r, uint8_t &g, uint8_t &b) {}
bool SenseCAP::isRGBOn() { return ledOn; }
void SenseCAP::angleToRGB(float angle, uint8_t &r, uint8_t &g, uint8_t &b) {}
void SenseCAP::encoderISR() {}
void SenseCAP::updateEncoder() {}
void SenseCAP::handleEncoderRGBChange(bool clockwise) {}
void SenseCAP::encoderISRStatic(void *arg) {}

// 处理滚轮按键用于屏幕控制
void SenseCAP::handleWheelButtonForScreen()
{
    bool btnState = getWheelButtonState();

    // 检测按键按下
    if (btnState && !screenBtnWasPressed)
    {
        screenBtnWasPressed = true;
        screenBtnPressTime = millis();
        screenBtnLongPressed = false;
    }
    // 检测按键释放
    else if (!btnState && screenBtnWasPressed)
    {
        screenBtnWasPressed = false;
        unsigned long pressDuration = millis() - screenBtnPressTime;

        // 短按（200ms-2s）: 唤醒屏幕
        if (pressDuration >= 200 && pressDuration < 2000)
        {
            Serial.println("短按：唤醒屏幕");
            screenAwake = true;
            // 实际唤醒屏幕操作：打开LCD电源并设置背光
            enableLCDPower();
            setBacklight(1023); // 设置最大亮度
        }
    }
    // 检测长按
    else if (btnState && screenBtnWasPressed && !screenBtnLongPressed)
    {
        unsigned long pressDuration = millis() - screenBtnPressTime;

        // 长按（2s以上）: 关闭屏幕并执行系统关机
        if (pressDuration >= 2000)
        {
            Serial.println("长按：关闭屏幕并执行系统关机");
            screenAwake = false;
            screenBtnLongPressed = true;
            // 实际关闭屏幕操作：关闭LCD电源并设置背光为0
            disableLCDPower();
            setBacklight(0); // 关闭背光
            // 执行系统关机
            systemPowerOff();
        }
    }
}
void SenseCAP::checkButtonInternal() {}
void SenseCAP::updateLedColor(bool reverse) {}

// 保留PA放大器控制函数，因为它们需要使用IO扩展功能
#include "Watcher_audio.h"

// ===== 音频控制功能实现（包装Watcher_audio类）=====

bool SenseCAP::initAudio()
{
    return watcherAudio.initAudio();
}

bool SenseCAP::initAudioAndPlay(const char *filename)
{
    return watcherAudio.initAudioAndPlay(filename);
}

bool SenseCAP::playAudioFile(const char *filename)
{
    return watcherAudio.playAudioFile(filename);
}

void SenseCAP::setAudioVolume(uint8_t volume)
{
    watcherAudio.setAudioVolume(volume);
}

void SenseCAP::setCodecVolume(uint8_t volume)
{
    watcherAudio.setCodecVolume(volume);
}

bool SenseCAP::enableAudioPA()
{
    if (!begin())
    {
        return false;
    }

    bool result = setPowerLevel(0x10, 1); // AUDIO_PA_EN = 0x10
    if (result)
    {
        watcherAudio.enableAudioPA();
        Serial.println("PA_EN引脚已拉高");
    }
    return result;
}

bool SenseCAP::disableAudioPA()
{
    bool result = setPowerLevel(0x10, 0); // AUDIO_PA_EN = 0x10
    if (result)
    {
        watcherAudio.disableAudioPA();
        Serial.println("PA_EN引脚已拉低");
    }
    return result;
}

void SenseCAP::audioLoop()
{
    watcherAudio.audioLoop();
}

bool SenseCAP::isAudioInitialized()
{
    return watcherAudio.isAudioInitialized();
}

// 全局函数实现 (向后兼容)
bool bsp_exp_io_set_level(uint8_t pin_mask, uint8_t level)
{
    return senseCAP.setPowerLevel(pin_mask, level);
}

bool enableGrovePower()
{
    return senseCAP.enableGrovePower();
}

bool disableGrovePower()
{
    return senseCAP.disableGrovePower();
}

bool beginWithGrovePower()
{
    return senseCAP.beginWithGrovePower();
}
