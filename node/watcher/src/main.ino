#include <Arduino.h>
#include <lvgl.h>
#include "Watcher_lvgl_init.h" // initHardware();
#include "generated/gui_guider.h"
lv_ui guider_ui;

#include <SenseCAP.h>
#include "MAX30102_sensor_functions.h"
char MAX30102Sensor = 0;

// 电池电量显示全局变量声明
lv_obj_t *battery_icon = nullptr;
lv_obj_t *percent_label = nullptr;

#include "fdrs_node_config.h"
#include <fdrs_node.h>

// ==================== Setup ====================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("  SenseCAP Watcher - Hello World");
  Serial.println("========================================\n");

  // 初始化完整的系统功能（包括电源管理和滚轮功能）
  if (!senseCAP.begin())
  {
    Serial.println("WARNING: Failed to initialize full system functionality");
  }
  else
  {
    Serial.println("Full system functionality initialized successfully");
  }

  // 初始化硬件
  initHardware(); // Watcher_lvgl_init.h

  // 初始化 LVGL
  Serial.println("初始化 LVGL...");
  lvglInit();
  Serial.println("LVGL 初始化完成\n");

  setup_ui(&guider_ui);

  // 初始显示"--"表示没有测量值
  lv_label_set_text(guider_ui.screen_label_1, "--");
  lv_label_set_text(guider_ui.screen_label_2, "--");
  // 隐藏"No Finger!"提示
  lv_obj_add_flag(guider_ui.screen_label_3, LV_OBJ_FLAG_HIDDEN);

  // 创建单个电池图标
  battery_icon = senseCAP.create_battery_icon_abs(guider_ui.screen, 75, 190, 10);

  // 创建一个电量百分比显示标签
  percent_label = lv_label_create(guider_ui.screen);
  lv_obj_align(percent_label, LV_ALIGN_TOP_LEFT, 140, 15);
  lv_obj_set_style_text_font(percent_label, &lv_font_montserratMedium_16, 0);
  lv_obj_set_style_text_color(percent_label, lv_color_white(), LV_PART_MAIN);
  lv_label_set_text(percent_label, " 75%");

  // 初始化MAX30102传感器
  if (initMAX30102Sensor())
  {
    Serial.println("MAX30102 initialized successfully.");
    MAX30102Sensor = 1;
  }
  else
  {
    String errorMsg = F("MAX30102 initialization failed. Stopping.");
    Serial.println(errorMsg);
    // 在屏幕上显示错误信息
    MAX30102Sensor = 0;
    lv_label_set_text(guider_ui.screen_label_4, "No MAX30102!");
  }

  senseCAP.playAudioFile("startup.mp3");

  beginFDRS();
}

// ==================== Loop ====================
void loop()
{
  // 检查滚轮按键状态（用于屏幕控制和关机功能）
  senseCAP.handleWheelButtonForScreen();

  // 传感器循环处理
  if (MAX30102Sensor)
    sensorLoop();

  senseCAP.audioLoop();

  static uint32_t last_tick = 0;
  uint32_t now = millis();

  // 更新 LVGL tick（首次初始化）
  if (last_tick == 0)
  {
    last_tick = now;
  }

  lv_tick_inc(now - last_tick);
  last_tick = now;

  // 更新显示
  if (num_fail < 10)
  {
    int avgHeartRate = getAverageHeartRate();
    int currentSPO2 = getCurrentSPO2();

    char hrText[10];
    char spo2Text[10];

    sprintf(hrText, "%3d", avgHeartRate);
    sprintf(spo2Text, "%3d", currentSPO2);

    if (validHeartRate && heartRate > 0)
    {
      // 发送心率数据到FDRS - 限制发送频率以避免干扰传感器读取
      static unsigned long lastHeartRateSend = 0;
      if (millis() - lastHeartRateSend > 1000)
      { // 每1秒发送一次
        int avgHeartRate = getAverageHeartRate();
        loadFDRS(avgHeartRate, CO2_T);
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

    if (validSPO2 && currentSPO2 > 0)
    {
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

    lv_label_set_text(guider_ui.screen_label_1, hrText);
    lv_label_set_text(guider_ui.screen_label_2, spo2Text);

    // 隐藏"No Finger!"提示
    lv_obj_add_flag(guider_ui.screen_label_3, LV_OBJ_FLAG_HIDDEN);
  }
  else
  {
    // 显示"--"表示没有测量值
    lv_label_set_text(guider_ui.screen_label_1, "--");
    lv_label_set_text(guider_ui.screen_label_2, "--");

    // 显示"No Finger!"提示
    lv_obj_clear_flag(guider_ui.screen_label_3, LV_OBJ_FLAG_HIDDEN);
  }

  // 定期更新电池状态（每1000毫秒更新一次）
  static uint32_t last_battery_update = 0;
  if (now - last_battery_update > 1000)
  {
    // 获取当前电池百分比
    int current_percent = senseCAP.getBatteryPercent();

    // 更新电池图标
    senseCAP.update_battery_icon(battery_icon, current_percent);

    // 更新电量百分比显示
    char percent_str[20];
    snprintf(percent_str, sizeof(percent_str), " %d%%", current_percent);
    lv_label_set_text(percent_label, percent_str);

    // 记录上次更新时间
    last_battery_update = now;
  }

  // 处理 LVGL 任务
  lvglLoop();
  delay(5);
}