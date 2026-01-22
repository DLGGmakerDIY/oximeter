#include <Arduino.h>
#include <lvgl.h>
#include "Watcher_lvgl_init.h" // 使用新的库头文件

#include <SenseCAP.h>

// 全局变量声明
lv_obj_t *battery_icon = nullptr;
lv_obj_t *percent_label = nullptr;

// ==================== Setup ====================
void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n========================================");
  Serial.println("  SenseCAP Watcher - Hello World");
  Serial.println("========================================\n");

  // 初始化硬件
  initHardware();

  // 初始化电源管理模块和旋转编码器
  // initWatcher();

  // 初始化SenseCAP实例的滚轮和按键功能
  senseCAP.begin();

  // 初始化 LVGL
  Serial.println("初始化 LVGL...");
  lvglInit();
  Serial.println("LVGL 初始化完成\n");

  // 获取活动屏幕并设置背景色为黑色
  lv_obj_t *screen = lv_scr_act();
  lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);

  // 创建单个电池图标（使用绝对位置，初始电量75%，位置x=100, y=100）
  battery_icon = senseCAP.create_battery_icon_abs(screen, 75, 100, 100);

  // 创建一个电量百分比显示标签
  percent_label = lv_label_create(screen);
  lv_obj_align(percent_label, LV_ALIGN_CENTER, 0, -40);
  lv_obj_set_style_text_font(percent_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(percent_label, lv_color_white(), LV_PART_MAIN);

  // 创建一个标签对象
  lv_obj_t *label = lv_label_create(screen);

  // 设置文本内容
  lv_label_set_text(label, "Battery Icons Demo");

  // 居中显示，位置稍微靠下
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 40);

  // 设置字体大小
  lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);

  // 设置文字颜色为白色
  lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);

  // 强制刷新屏幕，显示初始内容
  Serial.println("强制刷新LVGL屏幕...");
  lv_refr_now(NULL);

  Serial.println("\n初始化完成！");
  Serial.println("========================================");
}

// ==================== Loop ====================
void loop()
{
  // 检查滚轮按键状态（用于屏幕控制和关机功能）
  senseCAP.handleWheelButtonForScreen();

  static uint32_t last_tick = 0;
  uint32_t now = millis();

  // 更新 LVGL tick（首次初始化）
  if (last_tick == 0)
  {
    last_tick = now;
  }

  lv_tick_inc(now - last_tick);
  last_tick = now;

  // 定期更新电池状态（每500毫秒更新一次）
  static uint32_t last_battery_update = 0;
  if (now - last_battery_update > 500)
  {
    // 获取当前电池百分比
    int current_percent = senseCAP.getBatteryPercent();

    // 更新电池图标
    senseCAP.update_battery_icon(battery_icon, current_percent);

    // 更新电量百分比显示
    char percent_str[20]; // 增加缓冲区大小以避免溢出
    snprintf(percent_str, sizeof(percent_str), "Battery: %d%%", current_percent);
    lv_label_set_text(percent_label, percent_str);

    // 记录上次更新时间
    last_battery_update = now;
  }

  // 处理 LVGL 任务
  lvglLoop();
  delay(5);
}