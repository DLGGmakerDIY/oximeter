#include "Watcher_battery_icon.h"

// 创建全局实例
Watcher_battery_icon watcherBatteryIcon;

Watcher_battery_icon::Watcher_battery_icon()
{
    // 构造函数
}

int Watcher_battery_icon::get_battery_icon_index(int percentage)
{
    // 确保电量百分比在0-100范围内
    if (percentage < 0)
        percentage = 0;
    if (percentage > 100)
        percentage = 100;

    // 根据电量百分比选择图标
    int icon_index;
    if (percentage >= 80)
        icon_index = 0; // 满电
    else if (percentage >= 60)
        icon_index = 1; // 3/4
    else if (percentage >= 40)
        icon_index = 2; // 1/2
    else if (percentage >= 20)
        icon_index = 3; // 1/4
    else
        icon_index = 4; // 空电

    return icon_index;
}

lv_obj_t *Watcher_battery_icon::create_battery_icon(lv_obj_t *parent, int percentage, lv_align_t align, int x_offset, int y_offset)
{
    int icon_index = get_battery_icon_index(percentage);

    // 创建单个电池图标
    lv_obj_t *battery_icon = lv_label_create(parent);
    lv_label_set_text(battery_icon, battery_icons[icon_index]);
    lv_obj_align(battery_icon, align, x_offset, y_offset);
    lv_obj_set_style_text_color(battery_icon, battery_colors[icon_index], LV_PART_MAIN);
    lv_obj_set_style_text_font(battery_icon, &lv_font_montserrat_24, 0); // 使用更大的字体

    return battery_icon;
}

lv_obj_t *Watcher_battery_icon::create_battery_icon_abs(lv_obj_t *parent, int percentage, int x, int y)
{
    int icon_index = get_battery_icon_index(percentage);

    // 创建单个电池图标
    lv_obj_t *battery_icon = lv_label_create(parent);
    lv_label_set_text(battery_icon, battery_icons[icon_index]);
    lv_obj_set_pos(battery_icon, x, y); // 使用绝对位置
    lv_obj_set_style_text_color(battery_icon, battery_colors[icon_index], LV_PART_MAIN);
    lv_obj_set_style_text_font(battery_icon, &lv_font_montserrat_24, 0); // 使用更大的字体

    return battery_icon;
}

void Watcher_battery_icon::update_battery_icon(lv_obj_t *battery_icon, int percentage)
{
    if (!battery_icon) return;

    int icon_index = get_battery_icon_index(percentage);
    
    lv_label_set_text(battery_icon, battery_icons[icon_index]);
    lv_obj_set_style_text_color(battery_icon, battery_colors[icon_index], LV_PART_MAIN);
}
