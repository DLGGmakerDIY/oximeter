#include "Watcher_audio.h"
#include <Arduino.h>

// 音频相关头文件
#include "Audio.h"
#include "SPIFFS.h"
#include "es8311.h"

// 创建全局实例
Watcher_audio watcherAudio;

Watcher_audio::Watcher_audio()
{
    // 初始化成员变量
    audio = nullptr;
    es8311 = nullptr;
    audioInitialized = false;
    paEnEnabled = false;
}

bool Watcher_audio::initAudio()
{
    if (audioInitialized)
    {
        return true;
    }

    // 初始化SPIFFS
    if (!SPIFFS.begin())
    {
        Serial.println("SPIFFS Mount Failed");
        return false;
    }

    // 初始化音频对象
    if (audio == nullptr)
    {
        audio = new Audio();
    }

    // 初始化音频编解码器对象
    if (es8311 == nullptr)
    {
        es8311 = new ES8311();
    }

    // 设置I2S引脚
    audio->setPinout(AUDIO_I2S_BCLK, AUDIO_I2S_LRC, AUDIO_I2S_DOUT, AUDIO_I2S_MCLK);

    // 初始化ES8311编解码器
    if (!es8311->begin(AUDIO_I2C_SDA, AUDIO_I2C_SCL, 400000))
    {
        log_e("ES8311 begin failed");
        return false;
    }

    // 设置默认音量
    audio->setVolume(7); // default 0...21
    es8311->setVolume(80);
    es8311->setBitsPerSample(16);

    audioInitialized = true;
    return true;
}

bool Watcher_audio::initAudioAndPlay(const char *filename)
{
    // 初始化音频模块
    if (!initAudio())
    {
        return false;
    }

    // 播放音频文件
    return playAudioFile(filename);
}

bool Watcher_audio::playAudioFile(const char *filename)
{
    if (!audioInitialized)
    {
        if (!initAudio())
        {
            return false;
        }
    }

    // 连接并播放SPIFFS中的音频文件
    audio->connecttoFS(SPIFFS, filename);
    return true;
}

void Watcher_audio::setAudioVolume(uint8_t volume)
{
    if (audioInitialized && audio != nullptr)
    {
        audio->setVolume(volume);
    }
}

void Watcher_audio::setCodecVolume(uint8_t volume)
{
    if (audioInitialized && es8311 != nullptr)
    {
        es8311->setVolume(volume);
    }
}

// 注意：enableAudioPA和disableAudioPA函数需要通过IO扩展器控制，
// 因此将在SenseCAP类中实现为包装函数

bool Watcher_audio::enableAudioPA()
{
    // 这个函数将在SenseCAP类中实现，因为它需要控制IO扩展器
    paEnEnabled = true;
    return true;
}

bool Watcher_audio::disableAudioPA()
{
    // 这个函数将在SenseCAP类中实现，因为它需要控制IO扩展器
    paEnEnabled = false;
    return true;
}

void Watcher_audio::audioLoop()
{
    if (audioInitialized && audio != nullptr)
    {
        audio->loop();
    }
}

bool Watcher_audio::isAudioInitialized()
{
    return audioInitialized;
}