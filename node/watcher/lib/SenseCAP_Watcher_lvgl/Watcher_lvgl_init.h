#ifndef WATCHER_LVGL_INIT_H
#define WATCHER_LVGL_INIT_H

#include <Arduino.h>
#include <lvgl.h>
#include "Display_SPD2010.h"
#include "I2C_Driver.h"
#include <SenseCAP.h>

/**
 * @brief 硬件初始化函数
 *
 * 初始化I2C、PCA9535 IO扩展器、LCD及背光
 */
void initHardware();

/**
 * @brief Watcher系统初始化函数
 *
 * 初始化电源管理模块和旋转编码器
 */
void initWatcher();

/**
 * @brief LVGL初始化函数
 *
 * 初始化LVGL库、显示缓冲区和显示驱动
 */
void lvglInit();

/**
 * @brief LVGL主循环函数
 *
 * 处理LVGL的定时器和事件
 */
void lvglLoop();

#endif // WATCHER_LVGL_INIT_H
