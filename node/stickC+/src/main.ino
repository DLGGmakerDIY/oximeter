// 240x135
#include <M5Unified.h>
#include "MAX30102_sensor_functions.h"
#include "Free_Fonts.h" //  M5.Display.setFreeFont(FSS24);

#include "heartrate_hex.h"
#include "oxygen_hex.h"

#include "fdrs_node_config.h"
#include <fdrs_node.h>

// M5StickC Plus I2C引脚定义
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 26

// 创建M5Canvas对象用于显示数值（替代TFT_eSprite）
M5Canvas sprite = M5Canvas(&M5.Display);
M5Canvas labelSprite = M5Canvas(&M5.Display); // 用于显示标签的额外Sprite

void setup()
{
  Serial.begin(115200);
  auto cfg = M5.config();

  M5.begin(cfg);

  // 启用外部电源输出 (hat)
  M5.Power.setExtPower(true);

  M5.Display.setRotation(1); // 横屏 - 使用M5.Display

  // 初始化MAX30102传感器
  if (!initMAX30102Sensor())
  {
    Serial.println(F("MAX30102 initialization failed. Stopping."));
    while (1)
      ;
  }

  // 创建Canvas用于数值显示区域 (尺寸足够显示心率和血氧数值)
  sprite.createSprite(100, 30); // 增大Sprite尺寸以适应更大字体
  sprite.setFreeFont(FSS18);    // 使用更大字体显示数值

  // 创建Canvas用于标签显示
  labelSprite.createSprite(50, 25); // 为标签显示创建缓冲区
  labelSprite.setFreeFont(FSS9);

  M5.Display.setFreeFont(FSS9); // 设置标签字体
  M5.Display.setTextSize(1);    // 使用较小的标签字体大小

  // 显示初始界面，显示图标和标签
  M5.Display.drawBitmap(10, 0, heartrate.width, heartrate.height, (uint16_t *)heartrate.pixel_data, 1);

  // 使用Sprite显示HR标签
  labelSprite.fillSprite(BLACK);
  labelSprite.setTextColor(WHITE);
  labelSprite.setCursor(0, 0);
  labelSprite.printf("HR: ");
  labelSprite.pushSprite(80, 10); // 推送标签到屏幕，Y坐标调整以对齐

  // 显示初始数值 "--"
  sprite.fillSprite(BLACK);
  sprite.setTextColor(WHITE);
  sprite.setCursor(0, 0);
  sprite.printf("--");
  sprite.pushSprite(115, 8); // 数值显示在标签后，使用更大字体

  M5.Display.drawBitmap(10, 64, oxygen.width, oxygen.height, (uint16_t *)oxygen.pixel_data, 1);

  // 使用Sprite显示O2标签
  labelSprite.fillSprite(BLACK);
  labelSprite.setTextColor(WHITE);
  labelSprite.setCursor(0, 0);
  labelSprite.printf("O2: ");
  labelSprite.pushSprite(80, 70); // 推送标签到屏幕，Y坐标调整以对齐

  // 显示初始数值 "--%"
  sprite.fillSprite(BLACK);
  sprite.setTextColor(WHITE);
  sprite.setCursor(0, 0);
  sprite.printf("--%%");
  sprite.pushSprite(115, 68); // 数值显示在标签后，使用更大字体

  // 显示"put Finger"提示，使用较小字体并放在屏幕底部
  M5.Display.setFreeFont(FF10);       // 使用更小的字体显示提示信息
  M5.Display.setCursor(80 + 20, 110); // 放在屏幕底部中央位置
  M5.Display.setTextColor(WHITE, BLACK);
  M5.Display.printf("put Finger");
  M5.Display.setFreeFont(FSS18); // 恢复字体大小

  beginFDRS();
}

void loop()
{
  // M5.update();
  // delay(10);

  // 传感器循环处理
  sensorLoop();

  // Update display
  if (num_fail < 10)
  {
    int avgHeartRate = getAverageHeartRate();
    int currentSPO2 = getCurrentSPO2();

    // 更新心率数值部分（只更新数值，标签保持不变）
    sprite.fillSprite(BLACK); // 清除背景
    sprite.setTextColor(WHITE);
    sprite.setCursor(0, 0);
    if (validHeartRate && heartRate > 0 && num_fail < 10)
    {
      int displayHeartRate = avgHeartRate > 0 ? avgHeartRate : heartRate;
      sprite.printf("%d", displayHeartRate);

      // 发送心率数据到FDRS - 限制发送频率以避免干扰传感器读取
      static unsigned long lastHeartRateSend = 0;
      if (millis() - lastHeartRateSend > 1000)
      { // 每1秒发送一次
        loadFDRS(displayHeartRate, CO2_T);
        if (sendFDRSAsync()) // 使用异步发送避免阻塞
        {
          DBG("Heart Rate Data Sent Successfully!");
          lastHeartRateSend = millis();
        }
        else
        {
          DBG("Heart Rate Data Send Failed.");
        }
      }
    }
    else
    {
      sprite.printf("--");
    }
    sprite.pushSprite(115, 8); // 推送心率数值到屏幕标签后的位置

    // 更新血氧数值部分（只更新数值，标签保持不变）
    sprite.fillSprite(BLACK); // 清除背景
    sprite.setTextColor(WHITE);
    sprite.setCursor(0, 0);
    if (validSPO2 && currentSPO2 > 0 && num_fail < 10)
    {
      sprite.printf("%d%%", currentSPO2);

      // 发送血氧数据到FDRS - 限制发送频率以避免干扰传感器读取
      static unsigned long lastSPO2Send = 0;
      if (millis() - lastSPO2Send > 1000)
      { // 每1秒发送一次
        loadFDRS(currentSPO2, OXYGEN_T);
        if (sendFDRSAsync()) // 使用异步发送避免阻塞
        {
          DBG("SPO2 Data Sent Successfully!");
          lastSPO2Send = millis();
        }
        else
        {
          DBG("SPO2 Data Send Failed.");
        }
      }
    }
    else
    {
      sprite.printf("--");
    }
    sprite.pushSprite(115, 68); // 推送血氧数值到屏幕标签后的位置

    // 清除"No Finger!!"文本（如果之前显示过）
    M5.Display.fillRect(100, 110, 240, 20, BLACK);
  }
  else
  {
    // 显示默认心率数值
    sprite.fillSprite(BLACK); // 清除背景
    sprite.setTextColor(WHITE);
    sprite.setCursor(0, 0);
    sprite.printf("--");
    sprite.pushSprite(115, 8); // 推送心率数值到屏幕

    // 显示默认血氧数值
    sprite.fillSprite(BLACK); // 清除背景
    sprite.setTextColor(WHITE);
    sprite.setCursor(0, 0);
    sprite.printf("--");
    sprite.pushSprite(115, 68); // 推送血氧数值到屏幕

    // 显示"No Finger!"提示，使用较小字体并放在屏幕底部
    M5.Display.setFreeFont(FF10);       // 使用更小的字体显示提示信息
    M5.Display.setCursor(80 + 20, 110); // 放在屏幕底部中央位置
    M5.Display.setTextColor(WHITE, BLACK);
    M5.Display.printf("No Finger!");
    M5.Display.setFreeFont(FSS18); // 恢复字体大小
  }

  // Send samples and results to serial terminal (参考代码中的功能)
  // Serial.print(F("red="));
  // Serial.print(redBuffer[99], DEC);
  // Serial.print(F(", ir="));
  // Serial.print(irBuffer[99], DEC);

  // Serial.print(F(", HR="));
  // Serial.print(getAverageHeartRate(), DEC); // Using average heart rate

  // Serial.print(F(", HRvalid="));
  // Serial.print(validHeartRate, DEC);

  // Serial.print(F(", SPO2="));
  // Serial.print(getCurrentSPO2(), DEC);

  // Serial.print(F(", SPO2Valid="));
  // Serial.println(validSPO2, DEC);
}