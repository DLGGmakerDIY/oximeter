
#include <M5Unified.h>
#include "fdrs_gateway_config.h"
#include <fdrs_gateway.h>

void setup()
{
    Serial.begin(115200);
    auto cfg = M5.config();
    cfg.led_brightness = 128; // 设置亮度为50%（0-255）
    M5.begin(cfg);

    // 点亮LED
    M5.Led.setColor(0, 0x00FF00);

    beginFDRS();
}

void loop()
{
    loopFDRS();

    if (M5.BtnA.wasReleased())
    {
        Serial.println('A');
        // 点亮LED
        M5.Led.setColor(0, 0x00FF00);
        delay(500);
        M5.Led.setColor(0, 0x000000);
    }

    M5.update(); // Read the press state of the key
    delay(10);   // 添加小延迟以减少CPU使用率
}
