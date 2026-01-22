#include <Arduino.h>
#include <SenseCAP.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("SenseCAP Library Test");

  // 初始化电源管理模块
  if (!SenseCAP.begin()) {
    Serial.println("WARNING: Failed to initialize power management");
  } else {
    Serial.println("Power management initialized successfully");
  }

  // 只初始化旋转编码器，不初始化RGB灯
  if (!SenseCAP.initRotaryEncoder()) {
    Serial.println("WARNING: Failed to initialize rotary encoder");
  } else {
    Serial.println("Rotary encoder initialized successfully");
  }

  // 初始化背光
  if (!SenseCAP.initBacklight()) {
    Serial.println("WARNING: Failed to initialize backlight");
  } else {
    Serial.println("Backlight initialized successfully");
  }

  // 设置中等亮度背光
  SenseCAP.setBacklightMedium();
  Serial.println("Backlight set to medium brightness");

  // 打开Grove电源
  if (SenseCAP.enableGrovePower()) {
    Serial.println("Grove power enabled successfully");
  } else {
    Serial.println("WARNING: Failed to enable Grove power");
  }

  Serial.println("Setup complete");
}

void loop() {
  // 简单的测试程序，打印当前编码器位置
  Serial.print("Encoder Position: ");
  Serial.println(SenseCAP.getEncoderPosition());
  
  // 检查滚轮按键状态
  if (SenseCAP.getWheelButtonState()) {
    Serial.println("Wheel button pressed");
    // 短按切换背光亮度
    static int brightnessMode = 0;
    brightnessMode = (brightnessMode + 1) % 4;
    switch(brightnessMode) {
      case 0:
        SenseCAP.setBacklightOff();
        Serial.println("Backlight: Off");
        break;
      case 1:
        SenseCAP.setBacklight(256);
        Serial.println("Backlight: Low");
        break;
      case 2:
        SenseCAP.setBacklightMedium();
        Serial.println("Backlight: Medium");
        break;
      case 3:
        SenseCAP.setBacklightMax();
        Serial.println("Backlight: High");
        break;
    }
    while (SenseCAP.getWheelButtonState()) {
      // 等待按键释放
      delay(10);
    }
  }

  delay(500);
}
