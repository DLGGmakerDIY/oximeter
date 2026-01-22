#ifndef OXIMETER_DISPLAY_H
#define OXIMETER_DISPLAY_H

#include <M5Unified.h>
#include "Free_Fonts.h"

// 声明全局变量，用于在函数间共享Canvas对象
extern M5Canvas disdata;
extern M5Canvas label;

/**
 * @brief 初始化并显示血氧仪界面
 *
 * 此函数负责初始化血氧仪的显示界面，包括：
 * - 显示程序名称和启动信息
 * - 清屏并显示血氧和心率的图标
 * - 创建标签显示区域（HR和O2）
 * - 显示初始数值
 */
void initOximeterDisplay();

/**
 * @brief 更新血氧仪显示的数值
 *
 * 此函数负责更新血氧仪界面上指定设备的心率和血氧数值
 * 如果设备未激活，则显示"--"
 *
 * @param deviceIndex 设备索引（0表示第一个设备，1表示第二个设备）
 * @param hr 心率数值
 * @param o2 血氧数值
 * @param active 设备是否激活
 */
void updateOximeterDisplay(int deviceIndex, int hr, int o2, bool active);

#endif // OXIMETER_DISPLAY_H