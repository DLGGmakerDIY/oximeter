#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
import matplotlib.pyplot as plt
from datetime import datetime
import matplotlib.dates as mdates

# 设置matplotlib中文显示
plt.rcParams['font.sans-serif'] = ['Arial Unicode MS', 'SimHei', 'DejaVu Sans']
plt.rcParams['axes.unicode_minus'] = False

def load_json_data(file_path):
    """加载JSON数据文件"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            data = json.load(f)
        return data
    except FileNotFoundError:
        print("错误：未找到文件 {}".format(file_path))
        return None
    except json.JSONDecodeError:
        print("错误：{} 不是有效的JSON文件".format(file_path))
        return None

def plot_data(data):
    """绘制图表"""
    if not data:
        print("没有数据可绘制")
        return
    
    # 提取数据
    monitoring_time = [item['monitoringTime'] for item in data]
    local_time = [item['localTime'] for item in data]
    bpm_values = [item['BPM'] for item in data]
    spo2_values = [item['SpO2'] for item in data]
    
    # 过滤掉无效数据（-1表示无效）
    valid_indices = [i for i, bpm in enumerate(bpm_values) if bpm != -1]
    
    valid_time = [monitoring_time[i] for i in valid_indices]
    valid_local_time = [local_time[i] for i in valid_indices]
    valid_bpm = [bpm_values[i] for i in valid_indices]
    valid_spo2 = [spo2_values[i] for i in valid_indices]
    
    # 创建图形
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 10), sharex=True)
    
    # 绘制心率
    line1, = ax1.plot(valid_time, valid_bpm, 'r-', marker='o', linewidth=1, markersize=4, label='心率 (BPM)')
    ax1.set_ylabel('心率 (BPM)', color='red', fontsize=12)
    ax1.tick_params('y', colors='red', labelsize=10)
    ax1.grid(True, alpha=0.3, linestyle='--')
    ax1.legend(loc='upper left', fontsize=10)
    ax1.set_title('心率变化', fontsize=14, fontweight='bold', color='red', pad=10)
    
    # 设置心率范围
    ax1.set_ylim([30, 180])
    
    # 绘制血氧
    line2, = ax2.plot(valid_time, valid_spo2, 'b-', marker='o', linewidth=1, markersize=4, label='血氧 (%)')
    ax2.set_xlabel('监测时间 (秒)', fontsize=12, fontweight='bold')
    ax2.set_ylabel('血氧 (%)', color='blue', fontsize=12)
    ax2.tick_params('y', colors='blue', labelsize=10)
    ax2.grid(True, alpha=0.3, linestyle='--')
    ax2.legend(loc='upper left', fontsize=10)
    ax2.set_title('血氧变化', fontsize=14, fontweight='bold', color='blue', pad=20)
    
    # 设置血氧范围
    ax2.set_ylim([80, 100])
    
    # 标题
    fig.suptitle('ESP32 血氧心率监测数据图表', fontsize=18, fontweight='bold', y=0.98)
    
    # 添加交互式悬停提示
    from mpl_toolkits.axes_grid1 import host_subplot
    import mpl_toolkits.axisartist as AA
    
    # 创建悬停提示
    annot1 = ax1.annotate('', xy=(0,0), xytext=(-20,20), textcoords='offset points',
                        bbox=dict(boxstyle='round', fc='w', alpha=0.8),
                        arrowprops=dict(arrowstyle='->', alpha=0.5))
    annot1.set_visible(False)
    
    annot2 = ax2.annotate('', xy=(0,0), xytext=(-20,20), textcoords='offset points',
                        bbox=dict(boxstyle='round', fc='w', alpha=0.8),
                        arrowprops=dict(arrowstyle='->', alpha=0.5))
    annot2.set_visible(False)
    
    def update_annot(annot, line, ind, ax, values, ylabel, local_times):
        x, y = line.get_data()
        idx = ind['ind'][0]
        annot.xy = (x[idx], y[idx])
        # 将localTime（毫秒时间戳）转换为时分秒格式
        dt = datetime.fromtimestamp(local_times[idx] / 1000)
        time_str = dt.strftime('%H:%M:%S')
        text = "时间: {}\n{}: {:.1f}".format(time_str, ylabel, values[idx])
        annot.set_text(text)
        annot.get_bbox_patch().set_alpha(0.8)
    
    def hover(event):
        vis1 = annot1.get_visible()
        vis2 = annot2.get_visible()
        
        if event.inaxes == ax1:
            cont1, ind1 = line1.contains(event)
            if cont1:
                update_annot(annot1, line1, ind1, ax1, valid_bpm, '心率', valid_local_time)
                annot1.set_visible(True)
                fig.canvas.draw_idle()
            else:
                if vis1:
                    annot1.set_visible(False)
                    fig.canvas.draw_idle()
        
        if event.inaxes == ax2:
            cont2, ind2 = line2.contains(event)
            if cont2:
                update_annot(annot2, line2, ind2, ax2, valid_spo2, '血氧', valid_local_time)
                annot2.set_visible(True)
                fig.canvas.draw_idle()
            else:
                if vis2:
                    annot2.set_visible(False)
                    fig.canvas.draw_idle()
    
    fig.canvas.mpl_connect('motion_notify_event', hover)
    
    # 调整布局
    plt.subplots_adjust(left=0.1, right=0.95, top=0.92, bottom=0.1, hspace=0.3)
    
    # 启用工具栏（缩放、平移、保存等）
    plt.rcParams['toolbar'] = 'toolbar2'
    
    # 显示图表
    print("\n图表已打开！您可以：")
    print("- 使用鼠标滚轮缩放图表")
    print("- 点击并拖动平移图表")
    print("- 鼠标悬停在数据点上查看详细数值")
    print("- 使用窗口工具栏的各种功能")
    print("- 关闭窗口继续操作\n")
    
    plt.show()

def main():
    # 获取当前脚本所在目录
    current_dir = os.path.dirname(os.path.abspath(__file__))
    
    json_files = [f for f in os.listdir(current_dir) if f.endswith('.json')]
    
    if not json_files:
        print("在当前目录下未找到JSON文件")
        print("请将JSON文件放在: {}".format(current_dir))
        return
    
    print("找到以下JSON文件:")
    for i, file in enumerate(json_files):
        print("{}. {}".format(i + 1, file))
    
    # 选择文件
    if len(json_files) == 1:
        selected_file = json_files[0]
    else:
        try:
            choice = int(input("请选择要绘制的文件 (输入数字): "))
            if 1 <= choice <= len(json_files):
                selected_file = json_files[choice - 1]
            else:
                print("无效的选择")
                return
        except ValueError:
            print("请输入有效的数字")
            return
    
    file_path = os.path.join(current_dir, selected_file)
    print("\n正在加载: {}".format(file_path))
    
    # 加载数据
    data = load_json_data(file_path)
    
    if data:
        # 绘制图表
        plot_data(data)
        
        # 显示数据统计
        print("\n数据统计:")
        print("总数据点数: {}".format(len(data)))
        print("有效数据点数: {}".format(len([item for item in data if item['BPM'] != -1])))
        
        valid_bpm = [item['BPM'] for item in data if item['BPM'] != -1]
        if valid_bpm:
            print("平均心率: {:.1f} BPM".format(sum(valid_bpm) / len(valid_bpm)))
            print("最高心率: {} BPM".format(max(valid_bpm)))
            print("最低心率: {} BPM".format(min(valid_bpm)))
        
        valid_spo2 = [item['SpO2'] for item in data if item['SpO2'] != -1]
        if valid_spo2:
            print("平均血氧: {:.1f} %".format(sum(valid_spo2) / len(valid_spo2)))
            print("最高血氧: {} %".format(max(valid_spo2)))
            print("最低血氧: {} %".format(min(valid_spo2)))

if __name__ == "__main__":
    main()
