#include "OximeterDisplay.h"
#include "Free_Fonts.h"
#include "heartrate_hex.h"
#include "oxygen_hex.h"

// 定义全局变量
M5Canvas disdata = M5Canvas(&M5.Display);
M5Canvas label = M5Canvas(&M5.Display); // 用于显示标签的额外Sprite

/**
 * @brief 初始化并显示血氧仪界面
 *
 * 此函数负责初始化血氧仪的显示界面，包括：
 * - 显示程序名称和启动信息
 * - 清屏并显示血氧和心率的图标
 * - 创建标签显示区域（HR和O2）
 * - 显示初始数值
 */
void initOximeterDisplay()
{
    M5.Lcd.printf("test/prj/oximeter\n");
    Serial.printf("start.\n");

    // 首先清屏确保显示正常
    M5.Lcd.fillScreen(TFT_BLACK);

    // 绘制设备1的蓝色矩形框
    M5.Lcd.drawRect(10, 10, 150, 180, TFT_BLUE);

    // 绘制设备2的蓝色矩形框
    M5.Lcd.drawRect(160, 10, 150, 180, TFT_BLUE);

    // 显示设备1编号
    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(15, 15);
    M5.Lcd.print("1");

    // 显示设备2编号
    M5.Lcd.setTextColor(TFT_BLUE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(165, 15);
    M5.Lcd.print("2");

    // 显示初始界面，显示图标和标签
    M5.Display.drawBitmap(20, 30, heartrate.width, heartrate.height, (uint16_t *)heartrate.pixel_data, 1);
    M5.Display.drawBitmap(20, 120, oxygen.width, oxygen.height, (uint16_t *)oxygen.pixel_data, 1);
    M5.Display.drawBitmap(170, 30, heartrate.width, heartrate.height, (uint16_t *)heartrate.pixel_data, 1);
    M5.Display.drawBitmap(170, 120, oxygen.width, oxygen.height, (uint16_t *)oxygen.pixel_data, 1);

    // HR:创建Canvas用于标签显示
    label.createSprite(50, 25); // 为标签显示创建缓冲区
    label.setFreeFont(FSS9);
    label.fillSprite(BLACK);
    label.setTextColor(WHITE);
    label.setCursor(0, 0);
    label.printf("HR: ");
    label.pushSprite(90, 60); // 推送标签到屏幕，Y坐标调整以对齐

    // HR:创建Canvas用于标签显示
    label.createSprite(50, 25); // 为标签显示创建缓冲区
    label.setFreeFont(FSS9);
    label.fillSprite(BLACK);
    label.setTextColor(WHITE);
    label.setCursor(0, 0);
    label.printf("HR: ");
    label.pushSprite(240, 60); // 推送标签到屏幕，Y坐标调整以对齐

    // O2:创建Canvas用于标签显示
    label.createSprite(50, 25); // 为标签显示创建缓冲区
    label.setFreeFont(FSS9);
    label.fillSprite(BLACK);
    label.setTextColor(WHITE);
    label.setCursor(0, 0);
    label.printf("O2: ");
    label.pushSprite(90, 145); // 推送标签到屏幕，Y坐标调整以对齐

    // O2:创建Canvas用于标签显示
    label.createSprite(50, 25); // 为标签显示创建缓冲区
    label.setFreeFont(FSS9);
    label.fillSprite(BLACK);
    label.setTextColor(WHITE);
    label.setCursor(0, 0);
    label.printf("O2: ");
    label.pushSprite(240, 145); // 推送标签到屏幕，Y坐标调整以对齐

    // HR_1显示初始数值 "--"
    disdata.createSprite(35, 16); // 增大Sprite尺寸以适应更大字体
    disdata.setFreeFont(FSS9);    // 使用更大字体显示数值
    disdata.fillSprite(BLACK);
    disdata.setTextColor(WHITE);
    disdata.setCursor(0, 0);
    disdata.printf("--");
    disdata.pushSprite(120, 60); // 数值显示在标签后，使用更大字体

    // HR_2显示初始数值 "--"
    disdata.createSprite(35, 16); // 增大Sprite尺寸以适应更大字体
    disdata.setFreeFont(FSS9);    // 使用更大字体显示数值
    disdata.fillSprite(BLACK);
    disdata.setTextColor(WHITE);
    disdata.setCursor(0, 0);
    disdata.printf("--");
    disdata.pushSprite(270, 60); // 数值显示在标签后，使用更大字体

    // O2_1显示初始数值 "--"
    disdata.createSprite(35, 16); // 增大Sprite尺寸以适应更大字体
    disdata.setFreeFont(FSS9);    // 使用更大字体显示数值
    disdata.fillSprite(BLACK);
    disdata.setTextColor(WHITE);
    disdata.setCursor(0, 0);
    disdata.printf("--");
    disdata.pushSprite(120, 145); // 数值显示在标签后，使用更大字体

    // O2_2显示初始数值 "--"
    disdata.createSprite(35, 16); // 增大Sprite尺寸以适应更大字体
    disdata.setFreeFont(FSS9);    // 使用更大字体显示数值
    disdata.fillSprite(BLACK);
    disdata.setTextColor(WHITE);
    disdata.setCursor(0, 0);
    disdata.printf("--");
    disdata.pushSprite(270, 145); // 数值显示在标签后，使用更大字体
}

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
void updateOximeterDisplay(int deviceIndex, int hr, int o2, bool active)
{
    // 根据设备索引确定显示位置
    int hrX, o2X;
    if (deviceIndex == 0)
    {
        hrX = 120;
        o2X = 120;
    }
    else if (deviceIndex == 1)
    {
        hrX = 270;
        o2X = 270;
    }
    else
    {
        // 无效索引，不执行任何操作
        return;
    }

    // 更新心率数值
    disdata.createSprite(35, 16);
    disdata.setFreeFont(FSS9);
    disdata.fillSprite(BLACK);
    disdata.setTextColor(WHITE);
    disdata.setCursor(0, 0);
    if (!active)
    {
        disdata.printf("--");
    }
    else
    {
        disdata.printf("%d", hr);
    }
    disdata.pushSprite(hrX, 60);

    // 更新血氧数值
    disdata.createSprite(35, 16);
    disdata.setFreeFont(FSS9);
    disdata.fillSprite(BLACK);
    disdata.setTextColor(WHITE);
    disdata.setCursor(0, 0);
    if (!active)
    {
        disdata.printf("--");
    }
    else
    {
        disdata.printf("%d", o2);
    }
    disdata.pushSprite(o2X, 145);
}