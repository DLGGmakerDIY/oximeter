#include <Arduino.h>
#include <lvgl.h>
#include "Display_SPD2010.h"
#include "I2C_Driver.h"
#include "Watcher_lvgl_init.h"
#include "Touch_SPD2010.h"
#include <SenseCAP.h>

// ==================== 屏幕参数 ====================
#define LCD_WIDTH EXAMPLE_LCD_WIDTH
#define LCD_HEIGHT EXAMPLE_LCD_HEIGHT

// LVGL 缓冲区大小
#define LVGL_BUF_SIZE (LCD_WIDTH * LCD_HEIGHT / 10)

// ==================== 全局变量 ====================
// LVGL 显示缓冲区（使用PSRAM）
static lv_color_t *buf1 = NULL; // 动态分配到PSRAM
static lv_color_t *buf2 = NULL; // 动态分配到PSRAM

// 显示和输入设备相关
static lv_display_t *disp;
static lv_indev_t *indev_btn;
static lv_indev_t *indev_tp;

// 按钮状态
static bool btn_state = false;
static uint32_t last_btn_check = 0;
#define BUTTON_CHECK_INTERVAL 50 // 按钮检查间隔（毫秒）

// 触摸坐标
static lv_point_t touch_point = {0, 0};
static bool touch_state = false;

// ==================== 按钮读取函数 ====================
/**
 * @brief 读取按钮状态
 *
 * 从PCA9535 IO扩展器读取按钮状态
 *
 * @return true 按钮被按下
 * @return false 按钮未被按下
 */
bool read_button_state()
{
    // 读取TCA9554PWR Port 0的状态（按钮连接到Port 0）
    uint8_t port0_state = Read_EXIOS(TCA9554_INPUT_REG);

    // 假设按钮连接到P0.0（根据硬件实际连接调整）
    // TCA9554PWR输入为低电平表示按钮按下（通常按钮接地）
    return !(port0_state & 0x01);
}

// ==================== LVGL 输入设备回调 ====================
/**
 * @brief 触摸输入设备读取回调
 *
 * 向LVGL报告触摸坐标和状态
 *
 * @param indev 输入设备
 * @param data 输入数据
 */
void lvgl_touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    // 读取触摸状态
    uint16_t tp_x = 0;
    uint16_t tp_y = 0;
    uint8_t tp_cnt = 0;
    uint8_t touch_pressed = Touch_Get_xy(&tp_x, &tp_y, NULL, &tp_cnt, 1);

    // 更新触摸状态和坐标
    if (touch_pressed && tp_cnt > 0)
    {
        touch_state = true;
        touch_point.x = tp_x;
        touch_point.y = tp_y;
    }
    else
    {
        touch_state = false;
    }

    // 报告触摸状态
    data->state = touch_state ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

    // 报告触摸坐标
    data->point.x = touch_point.x;
    data->point.y = touch_point.y;
}

/**
 * @brief 按钮输入设备读取回调
 *
 * 向LVGL报告按钮状态
 *
 * @param indev 输入设备
 * @param data 输入数据
 */
void lvgl_btn_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    uint32_t now = millis();

    // 定期检查按钮状态
    if (now - last_btn_check >= BUTTON_CHECK_INTERVAL)
    {
        btn_state = read_button_state();
        last_btn_check = now;
    }

    // 报告按钮状态
    data->state = btn_state ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

    // 对于按钮输入，不需要坐标
    data->point.x = 0;
    data->point.y = 0;
}

// ==================== LVGL 回调函数 ====================
// 刷新显示回调
void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    LCD_addWindow(area->x1, area->y1, area->x2, area->y2, (uint16_t *)px_map);
    lv_display_flush_ready(disp);
}

// ==================== 硬件初始化函数 ====================
void initHardware()
{
    Serial.println("[0/3] 初始化 I2C0 总线...");
    I2C_Init();
    Serial.println("      I2C0 总线初始化完成");

    Serial.println("[0/3] 初始化 I2C1 总线...");
    I2C1_Init();
    Serial.println("      I2C1 总线初始化完成");

    Serial.println("[1/3] 初始化 PCA9535 (16位IO扩展器)...");
    // 初始化IO扩展器，配置IO模式
    // P0.0设为输入（按钮），其余设为输出
    // 0x0001 = 0000 0000 0000 0001，P0.0设为输入，其余设为输出
    PCA9535_Init(0x0001);
    Serial.println("      PCA9535 配置完成");

    Serial.println("[2/3] 使能LCD电源 (PCA9535 Port1)...");
    // 首先使能SYSTEM电源 (P1.2 = bit 2)
    Set_EXIO(11, High); // P1.2 对应引脚11（PORT1从引脚9开始计数，P1.0=9，P1.1=10，P1.2=11）
    delay(100);

    // 然后使能LCD电源 (P1.1 = bit 1)
    Set_EXIO(10, High); // P1.1 对应引脚10
    delay(100);
    delay(100);
    Serial.println("      LCD电源已使能");

    // 初始化 LCD
    Serial.println("[4/4] 初始化 LCD...");
    LCD_Init();
    Serial.println("      LCD 初始化完成");

    Serial.println("[5/5] 初始化背光...");
    Backlight_Init();
    Set_Backlight(80);
    Serial.println("      背光初始化完成");

    // 初始化触摸控制器
    Serial.println("[6/6] 初始化触摸控制器...");
    // 使用系统预定义的Wire1实例，避免创建新的I2C实例导致冲突
    if (Touch_Init(&Wire1))
    {
        Serial.println("      触摸控制器初始化完成");
    }
    else
    {
        Serial.println("      触摸控制器初始化失败");
    }
}

// ==================== LVGL 初始化函数 ====================
void lvglInit()
{
    Serial.println("  [1/7] 分配PSRAM缓冲区...");
    // 分配LVGL缓冲区到PSRAM
    buf1 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);
    buf2 = (lv_color_t *)heap_caps_malloc(LVGL_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_SPIRAM);

    if (buf1 == NULL || buf2 == NULL)
    {
        Serial.println("  ❌ PSRAM缓冲区分配失败！");
        while (1)
        {
            delay(1000);
        }
    }
    Serial.printf("  ✓ PSRAM缓冲区分配成功 (每个 %d bytes)\n", LVGL_BUF_SIZE * sizeof(lv_color_t));

    Serial.println("  [2/7] 调用 lv_init()...");
    // 初始化 LVGL
    lv_init();
    Serial.println("  [3/7] lv_init() 完成");

    Serial.println("  [4/7] 创建显示...");
    // 创建显示设备
    disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    if (disp == NULL)
    {
        Serial.println("  ❌ 显示设备创建失败！");
        while (1)
        {
            delay(1000);
        }
    }

    Serial.println("  [5/7] 配置显示参数...");
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
    lv_display_set_buffers(disp, buf1, buf2, LVGL_BUF_SIZE * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);

    Serial.println("  [6/7] 注册按钮输入设备...");
    // 创建按钮输入设备
    indev_btn = lv_indev_create();
    lv_indev_set_type(indev_btn, LV_INDEV_TYPE_BUTTON);
    lv_indev_set_read_cb(indev_btn, lvgl_btn_read_cb);
    lv_indev_set_display(indev_btn, disp);

    Serial.println("  [7/7] 注册触摸输入设备...");
    // 创建触摸输入设备
    indev_tp = lv_indev_create();
    lv_indev_set_type(indev_tp, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_tp, lvgl_touch_read_cb);
    lv_indev_set_display(indev_tp, disp);

    Serial.println("  ✓ 输入设备注册完成");
}

// ==================== Watcher 系统初始化函数 ====================
void initWatcher()
{
    // 初始化电源管理模块
    if (!senseCAP.begin())
    {
        Serial.println("WARNING: Failed to initialize power management");
    }
    else
    {
        Serial.println("Power management initialized successfully");
    }

    // 只初始化旋转编码器，不初始化RGB灯
    if (!senseCAP.initRotaryEncoder())
    {
        Serial.println("WARNING: Failed to initialize rotary encoder");
    }
    else
    {
        Serial.println("Rotary encoder initialized successfully");
    }
}

// ==================== LVGL 循环函数 ====================
void lvglLoop()
{
    lv_timer_handler();
}