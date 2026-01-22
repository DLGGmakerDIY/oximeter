#ifndef WATCHER_AUDIO_H
#define WATCHER_AUDIO_H

#include <Arduino.h>

// 音频相关前向声明
class Audio;
class ES8311;

// 音频相关引脚定义
#define AUDIO_I2S_DOUT 16
#define AUDIO_I2S_BCLK 11
#define AUDIO_I2S_MCLK 10
#define AUDIO_I2S_LRC 12
#define AUDIO_I2C_SDA 47
#define AUDIO_I2C_SCL 48
#define AUDIO_PA_EN 0x10 // PA_EN引脚（IO扩展芯片的P14引脚，对应Port 1的第4位）

/**
 * @brief SenseCAP Watcher 音频管理类
 * 
 * 用于控制音频播放、音量调节和功率放大器等功能
 */
class Watcher_audio
{
private:
    Audio *audio;
    ES8311 *es8311;
    bool audioInitialized;
    bool paEnEnabled; // PA_EN使能状态

public:
    /**
     * @brief 构造函数
     */
    Watcher_audio();
    
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
};

// 创建全局实例
extern Watcher_audio watcherAudio;

#endif // WATCHER_AUDIO_H