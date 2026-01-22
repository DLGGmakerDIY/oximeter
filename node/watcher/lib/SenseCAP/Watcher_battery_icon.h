#ifndef WATCHER_BATTERY_ICON_H
#define WATCHER_BATTERY_ICON_H

#include <lvgl.h>

/**
 * @brief SenseCAP Watcher 电池图标类
 * 
 * 用于创建和显示LVGL电池图标
 */
class Watcher_battery_icon
{
public:
    /**
     * @brief 构造函数
     */
    Watcher_battery_icon();
    
    /**
     * @brief 创建电池图标（使用对齐方式）
     * @param parent 父对象指针（通常是屏幕）
     * @param percentage 电量百分比（0-100）
     * @param align 对齐方式（LV_ALIGN_*）
     * @param x_offset x轴偏移量
     * @param y_offset y轴偏移量
     * @return 创建的电池图标对象指针
     */
    lv_obj_t *create_battery_icon(lv_obj_t *parent, int percentage, lv_align_t align, int x_offset, int y_offset);
    
    /**
     * @brief 创建电池图标（使用绝对位置）
     * @param parent 父对象指针（通常是屏幕）
     * @param percentage 电量百分比（0-100）
     * @param x 屏幕x坐标（左上角为原点）
     * @param y 屏幕y坐标（左上角为原点）
     * @return 创建的电池图标对象指针
     */
    lv_obj_t *create_battery_icon_abs(lv_obj_t *parent, int percentage, int x, int y);
    
    /**
     * @brief 更新电池图标
     * @param battery_icon 电池图标对象指针
     * @param percentage 电量百分比（0-100）
     */
    void update_battery_icon(lv_obj_t *battery_icon, int percentage);
    
    /**
     * @brief 根据电量获取图标索引
     * @param percentage 电量百分比（0-100）
     * @return 图标索引
     */
    int get_battery_icon_index(int percentage);
    
private:
    // 创建电池图标样式数组
    const char *battery_icons[5] = {
        LV_SYMBOL_BATTERY_FULL,
        LV_SYMBOL_BATTERY_3,
        LV_SYMBOL_BATTERY_2,
        LV_SYMBOL_BATTERY_1,
        LV_SYMBOL_BATTERY_EMPTY
    };
    
    // 对应的颜色数组
    lv_color_t battery_colors[5] = {
        lv_color_make(10, 179, 0),  // 绿色
        lv_color_make(10, 179, 0),  // 绿色
        lv_color_make(255, 153, 0), // 橙色
        lv_color_make(255, 0, 0),   // 红色
        lv_color_make(255, 0, 0)    // 红色
    };
};

// 创建全局实例
extern Watcher_battery_icon watcherBatteryIcon;

#endif // WATCHER_BATTERY_ICON_H
