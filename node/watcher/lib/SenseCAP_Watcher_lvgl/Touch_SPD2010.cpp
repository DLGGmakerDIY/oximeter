#include "Touch_SPD2010.h"
#include <Wire.h>

// 创建触摸专用的I2C总线实例 - 与成功扫描程序使用相同的方式
#define TOUCH_SDA 39
#define TOUCH_SCL 38
#define TOUCH_CLOCK 400000

// 声明I2C实例指针，默认使用内部创建的实例
TwoWire *I2CTouchPtr = nullptr;

// 内部I2C实例，仅当外部未提供时使用
TwoWire I2CTouchInternal = TwoWire(1); // 使用I2C端口1

struct SPD2010_Touch touch_data = {0};
uint8_t Touch_interrupts = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool I2C_Read_Touch(uint8_t Driver_addr, uint16_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
  // 使用I2C_Driver提供的I2C1_Read16函数进行16位寄存器地址的I2C读取
  return I2C1_Read16(Driver_addr, Reg_addr, Reg_data, Length);
}
bool I2C_Write_Touch(uint8_t Driver_addr, uint16_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
  // 使用I2C_Driver提供的I2C1_Write16函数进行16位寄存器地址的I2C写入
  return I2C1_Write16(Driver_addr, Reg_addr, Reg_data, Length);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IRAM_ATTR Touch_SPD2010_ISR(void)
{
  Touch_interrupts = true;
}
// 新的初始化函数，允许接受外部提供的I2C实例
uint8_t Touch_Init(TwoWire *externalI2C)
{
  Serial.println("Starting Touch_Init()...");

  // 设置I2C实例指针
  if (externalI2C != nullptr)
  {
    I2CTouchPtr = externalI2C;
    Serial.println("Using external I2C instance provided");
  }
  else
  {
    // 如果没有提供外部实例，使用内部创建的实例
    I2CTouchPtr = &I2CTouchInternal;
    Serial.println("Initializing touch I2C bus on SDA=39, SCL=38...");

    // 确保总线完全关闭后再初始化
    I2CTouchPtr->end();
    delay(100); // 延长延迟时间，确保总线完全重置

    // 与成功扫描程序使用完全相同的时钟频率 (400kHz)
    I2CTouchPtr->begin(TOUCH_SDA, TOUCH_SCL, 400000);
  }

  // 验证I2C总线是否真的初始化成功
  Serial.println("Touch I2C bus initialized");

  // 检查0x53地址的设备是否存在
  bool deviceFound = false;

  Serial.println("Starting I2C device detection for address 0x53...");

  // 使用现有的I2C实例进行扫描，避免创建多个实例导致冲突
  Serial.println("Scanning I2C bus (SDA=39, SCL=38)...");
  uint8_t cnt = 0;

  // 直接使用现有的I2CTouch实例进行扫描，与成功扫描程序完全相同的逻辑
  for (uint8_t addr = 1; addr < 127; addr++)
  {
    I2CTouchPtr->beginTransmission(addr);
    // 不写入任何数据，直接发送地址并检查ACK响应
    uint8_t ec = I2CTouchPtr->endTransmission(true);
    if (ec == 0)
    {
      Serial.printf("Found I2C device at address 0x%02X\n", addr);
      cnt++;
      if (addr == 0x53)
      {
        deviceFound = true;
        Serial.println("Found target device at address 0x53!");
        break; // 找到目标设备后立即退出循环
      }
    }
    else if (addr == 0x53)
    {
      Serial.printf("No ACK at address 0x53, error code: %d\n", ec);
    }
  }

  Serial.printf("I2C scan complete. Found %d devices.\n", cnt);

  // 如果找到了设备，使用原始的I2CTouch实例再次确认
  if (deviceFound)
  {
    Serial.println("Confirming device with original I2CTouch instance...");
    I2CTouchPtr->beginTransmission(0x53);
    uint8_t result = I2CTouchPtr->endTransmission();
    if (result == 0)
    {
      Serial.println("Device confirmation successful!");
    }
    else
    {
      Serial.printf("Device confirmation failed with error: %d\n", result);
      // 即使确认失败，我们也认为设备存在，因为独立扫描器找到了它
      Serial.println("Note: Independent scanner found device, continuing...");
      deviceFound = true;
    }
  }

  if (!deviceFound)
  {
    Serial.println("I2C device NOT found at address 0x53");

    // 检查PCA9535是否正确初始化（确保电源控制正常）
    Serial.println("Checking PCA9535 status...");
    uint8_t port1_status;

    // 使用I2C_Driver的函数和互斥锁保护I2C总线访问
    if (I2C_Read(0x21, 0x03, &port1_status, 1))
    {
      Serial.printf("PCA9535 Port1 status: 0x%02X\n", port1_status);
      Serial.printf("LCD Power (P1.1): %s\n", (port1_status & 0x02) ? "ON" : "OFF");
    }

    // 尝试扫描整个I2C总线，查看是否有其他设备
    Serial.println("Scanning I2C bus (SDA=39, SCL=38)...");
    int deviceCount = 0;
    for (uint8_t addr = 1; addr < 127; addr++)
    {
      uint8_t result;
      I2CTouchPtr->beginTransmission(addr);
      result = I2CTouchPtr->endTransmission();
      if (result == 0)
      {
        Serial.printf("Found I2C device at address 0x%02X\n", addr);
        deviceCount++;
      }
      else if (result != 2) // 2 = NACK on transmit of address
      {
        Serial.printf("Error at address 0x%02X: %d\n", addr, result);
      }
    }
    Serial.printf("I2C scan complete. Found %d devices.\n", deviceCount);
  }

  // 只有在设备找到时才调用配置函数
  if (deviceFound)
  {
    Serial.println("Calling SPD2010_Read_cfg()...");
    SPD2010_Read_cfg();

    // 重新启用触摸中断配置，使用FALLING触发方式
    pinMode(EXAMPLE_PIN_NUM_TOUCH_INT, INPUT_PULLUP);
    // Serial.printf("DEBUG: Interrupt pin %d initial state: %d\n", EXAMPLE_PIN_NUM_TOUCH_INT, digitalRead(EXAMPLE_PIN_NUM_TOUCH_INT));
    attachInterrupt(EXAMPLE_PIN_NUM_TOUCH_INT, Touch_SPD2010_ISR, FALLING);

    // 尝试手动触发一次触摸检测
    // Serial.println("DEBUG: Manually triggering touch detection...");
    tp_status_t tp_status; // 声明tp_status变量
    read_tp_status_length(&tp_status);
    // Serial.printf("DEBUG: Post-init TP Status - cpu_run:%d, tic_in_bios:%d, tic_in_cpu:%d, pt_exist:%d, gesture:%d, read_len:%d\n",
    // tp_status.status_high.cpu_run,
    // tp_status.status_high.tic_in_bios,
    // tp_status.status_high.tic_in_cpu,
    // tp_status.status_low.pt_exist,
    // tp_status.status_low.gesture,
    // tp_status.read_len);

    return 1; // 初始化成功
  }
  else
  {
    Serial.println("Skipping SPD2010_Read_cfg() because device not found");
    return 0; // 初始化失败
  }
}
/* Reset controller */
uint8_t SPD2010_Touch_Reset(void)
{
  Set_EXIO(EXIO_PIN1, Low);
  delay(50);
  Set_EXIO(EXIO_PIN1, High);
  delay(50);
  return true;
}

uint16_t SPD2010_Read_cfg(void)
{
  read_fw_version();

  // 添加初始化命令序列，与参考项目保持一致
  tp_status_t tp_status = {0};
  read_tp_status_length(&tp_status);

  if (tp_status.status_high.tic_in_bios)
  {
    /* Write Clear TINT Command */
    write_tp_clear_int_cmd();
    /* Write CPU Start Command */
    write_tp_cpu_start_cmd();
  }
  else if (tp_status.status_high.tic_in_cpu)
  {
    /* Write Touch Change Command */
    write_tp_point_mode_cmd();
    /* Write Touch Start Command */
    write_tp_start_cmd();
    /* Write Clear TINT Command */
    write_tp_clear_int_cmd();
  }
  else if (tp_status.status_high.cpu_run && tp_status.read_len == 0)
  {
    write_tp_clear_int_cmd();
  }

  return 1;
}
// reads sensor and touches
// updates Touch Points
void Touch_Read_Data(void)
{
  uint8_t touch_cnt = 0;
  struct SPD2010_Touch touch = {0};
  tp_read_data(&touch);

  noInterrupts();
  /* Expect Number of touched points */
  touch_cnt = (touch.touch_num > CONFIG_ESP_LCD_TOUCH_MAX_POINTS ? CONFIG_ESP_LCD_TOUCH_MAX_POINTS : touch.touch_num);
  touch_data.touch_num = touch_cnt;
  /* Fill all coordinates */
  for (int i = 0; i < touch_cnt; i++)
  {
    touch_data.rpt[i].x = touch.rpt[i].x;
    touch_data.rpt[i].y = touch.rpt[i].y;
    touch_data.rpt[i].weight = touch.rpt[i].weight;
  }
  interrupts();
}
bool Touch_Get_xy(uint16_t *x, uint16_t *y, uint16_t *strength, uint8_t *point_num, uint8_t max_point_num)
{
  /* Read data from touch controller */
  Touch_Read_Data();

  // 添加调试信息
  // Serial.printf("DEBUG: Touch_Get_xy called. touch_data.touch_num = %d\n", touch_data.touch_num);
  // for (size_t i = 0; i < touch_data.touch_num; i++)
  // {
  //   Serial.printf("DEBUG: Touch point %d - x:%d, y:%d, weight:%d\n", i,
  //                 touch_data.rpt[i].x, touch_data.rpt[i].y, touch_data.rpt[i].weight);
  // }

  /* Count of points */
  *point_num = (touch_data.touch_num > max_point_num ? max_point_num : touch_data.touch_num);
  for (size_t i = 0; i < *point_num; i++)
  {
    // 直接使用原始坐标，与参考项目一致
    x[i] = touch_data.rpt[i].x;
    y[i] = touch_data.rpt[i].y;

    // 确保坐标在有效范围内（0-4095）
    if (x[i] > 4095)
      x[i] = 4095;
    if (y[i] > 4095)
      y[i] = 4095;

    if (strength)
    {
      strength[i] = touch_data.rpt[i].weight;
    }

    // 打印坐标信息
    printf("Touch position: %d,%d\r\n", x[i], y[i]);
  }

  // 打印要返回的数据
  // Serial.printf("DEBUG: Returning %d touch points\n", *point_num);
  // if (*point_num > 0)
  // {
  //   Serial.printf("DEBUG: Returning x:%d, y:%d\n", x[0], y[0]);
  // }

  /* Clear available touch points count */
  touch_data.touch_num = 0;
  return (*point_num > 0);
  ;
}

void example_touchpad_read(void)
{
  bool tp_pressed = false;
  uint16_t tp_x = 0;
  uint16_t tp_y = 0;
  uint8_t tp_cnt = 0;
  /* Read data from touch controller */
  tp_pressed = Touch_Get_xy(&tp_x, &tp_y, NULL, &tp_cnt, 1);
  if (tp_pressed && (tp_cnt > 0))
  {
    printf("Touch position: %d,%d\r\n", tp_x, tp_y);
  }
  else
  {
    // data->state = LV_INDEV_STATE_REL;
  }
}
void Touch_Loop(void)
{
  example_touchpad_read();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
esp_err_t write_tp_point_mode_cmd()
{
  uint8_t sample_data[4];
  sample_data[0] = 0x50;
  sample_data[1] = 0x00;
  sample_data[2] = 0x00;
  sample_data[3] = 0x00;
  I2C_Write_Touch(SPD2010_ADDR, (((uint16_t)sample_data[0] << 8) | (sample_data[1])), &sample_data[2], 2);
  esp_rom_delay_us(200);
  return ESP_OK;
}

esp_err_t write_tp_start_cmd()
{
  uint8_t sample_data[4];
  sample_data[0] = 0x46;
  sample_data[1] = 0x00;
  sample_data[2] = 0x00;
  sample_data[3] = 0x00;
  I2C_Write_Touch(SPD2010_ADDR, (((uint16_t)sample_data[0] << 8) | (sample_data[1])), &sample_data[2], 2);
  esp_rom_delay_us(200);
  return ESP_OK;
}

esp_err_t write_tp_cpu_start_cmd()
{
  uint8_t sample_data[4];
  sample_data[0] = 0x04;
  sample_data[1] = 0x00;
  sample_data[2] = 0x01;
  sample_data[3] = 0x00;
  I2C_Write_Touch(SPD2010_ADDR, (((uint16_t)sample_data[0] << 8) | (sample_data[1])), &sample_data[2], 2);
  esp_rom_delay_us(200);
  return ESP_OK;
}

esp_err_t write_tp_clear_int_cmd()
{
  uint8_t sample_data[4];
  sample_data[0] = 0x02;
  sample_data[1] = 0x00;
  sample_data[2] = 0x01;
  sample_data[3] = 0x00;
  I2C_Write_Touch(SPD2010_ADDR, (((uint16_t)sample_data[0] << 8) | (sample_data[1])), &sample_data[2], 2);
  esp_rom_delay_us(200);
  return ESP_OK;
}

esp_err_t read_tp_status_length(tp_status_t *tp_status)
{
  uint8_t sample_data[4];
  sample_data[0] = 0x20;
  sample_data[1] = 0x00;
  I2C_Read_Touch(SPD2010_ADDR, (((uint16_t)sample_data[0] << 8) | (sample_data[1])), sample_data, 4);
  esp_rom_delay_us(200);

  // 打印原始状态数据
  // Serial.printf("DEBUG: Raw status data: 0x%02X 0x%02X 0x%02X 0x%02X\n",
  //               sample_data[0], sample_data[1], sample_data[2], sample_data[3]);
  tp_status->status_low.pt_exist = (sample_data[0] & 0x01);
  tp_status->status_low.gesture = (sample_data[0] & 0x02);
  tp_status->status_low.aux = ((sample_data[0] & 0x08)); // aux, cytang
  tp_status->status_high.tic_busy = ((sample_data[1] & 0x80) >> 7);
  tp_status->status_high.tic_in_bios = ((sample_data[1] & 0x40) >> 6);
  tp_status->status_high.tic_in_cpu = ((sample_data[1] & 0x20) >> 5);
  tp_status->status_high.tint_low = ((sample_data[1] & 0x10) >> 4);
  tp_status->status_high.cpu_run = ((sample_data[1] & 0x08) >> 3);

  // 尝试不同的字节顺序解析read_len，并添加合理性检查
  uint16_t len1 = (sample_data[3] << 8 | sample_data[2]); // 当前解析方式
  uint16_t len2 = (sample_data[2] << 8 | sample_data[3]); // 交换字节顺序

  // 选择合理的read_len值
  if (len1 > 100 && len2 <= 100)
  {
    tp_status->read_len = len2;
    // Serial.printf("DEBUG: Using swapped byte order for read_len: %d\n", tp_status->read_len);
  }
  else if (len1 <= 100)
  {
    tp_status->read_len = len1;
  }
  else
  {
    // 两个值都不合理，设置默认值
    tp_status->read_len = 10;
    // Serial.printf("DEBUG: Both read_len values are unreasonable, using default: %d\n", tp_status->read_len);
  }

  return ESP_OK;
}

esp_err_t read_tp_hdp(tp_status_t *tp_status, SPD2010_Touch *touch)
{
  uint8_t sample_data[4 + (10 * 6)]; // 4 Bytes Header + 10 Finger * 6 Bytes
  uint8_t i, offset;
  uint8_t check_id;

  // 与参考项目一致，设置命令为0x00, 0x03
  sample_data[0] = 0x00;
  sample_data[1] = 0x03;

  // Serial.printf("DEBUG: read_tp_hdp called. read_len:%d\n", tp_status->read_len);

  // 使用tp_status->read_len直接读取，与参考项目一致
  I2C_Read_Touch(SPD2010_ADDR, (((uint16_t)sample_data[0] << 8) | (sample_data[1])), sample_data, tp_status->read_len);

  // 打印所有读取到的采样数据
  // Serial.printf("DEBUG: Sample data (%d bytes): ", tp_status->read_len);
  // for (int j = 0; j < tp_status->read_len; j++)
  // {
  //   Serial.printf("0x%02X ", sample_data[j]);
  // }
  // Serial.println();

  check_id = sample_data[4];
  // Serial.printf("DEBUG: check_id:0x%02X, pt_exist:%d\n", check_id, tp_status->status_low.pt_exist);

  if ((check_id <= 0x0A) && tp_status->status_low.pt_exist)
  {
    // 计算触摸点数量，与参考项目一致
    touch->touch_num = ((tp_status->read_len - 4) / 6);
    // Serial.printf("DEBUG: Setting touch_num to %d\n", touch->touch_num);
    touch->gesture = 0x00;

    // 确保触摸点数量不超过最大值
    if (touch->touch_num > 10)
    {
      touch->touch_num = 10;
      // Serial.printf("DEBUG: Touch_num exceeds maximum, limiting to 10\n");
    }

    for (i = 0; i < touch->touch_num; i++)
    {
      offset = i * 6;
      touch->rpt[i].id = sample_data[4 + offset];
      // 与参考项目一致的坐标解析方式
      touch->rpt[i].x = (((sample_data[7 + offset] & 0xF0) << 4) | sample_data[5 + offset]);
      touch->rpt[i].y = (((sample_data[7 + offset] & 0x0F) << 8) | sample_data[6 + offset]);
      touch->rpt[i].weight = sample_data[8 + offset];

      // 确保坐标值在有效范围内
      if (touch->rpt[i].x > 4095)
        touch->rpt[i].x = 4095;
      if (touch->rpt[i].y > 4095)
        touch->rpt[i].y = 4095;

      // Serial.printf("DEBUG: Touch point %d - id:0x%02X, x:%d, y:%d, weight:%d\n",
      //               i, touch->rpt[i].id, touch->rpt[i].x, touch->rpt[i].y, touch->rpt[i].weight);
    }
    /* For slide gesture recognize */
    if ((touch->rpt[0].weight != 0) && (touch->down != 1))
    {
      touch->down = 1;
      touch->up = 0;
      touch->down_x = touch->rpt[0].x;
      touch->down_y = touch->rpt[0].y;
    }
    else if ((touch->rpt[0].weight == 0) && (touch->down == 1))
    {
      touch->up = 1;
      touch->down = 0;
      touch->up_x = touch->rpt[0].x;
      touch->up_y = touch->rpt[0].y;
    }
    /* Dump Log */
    // for (uint8_t finger_num = 0; finger_num < touch->touch_num; finger_num++) {
    //   printf("ID[%d], X[%d], Y[%d], Weight[%d]\n",
    //                 touch->rpt[finger_num].id,
    //                 touch->rpt[finger_num].x,
    //                 touch->rpt[finger_num].y,
    //                 touch->rpt[finger_num].weight);
    // }
  }
  else if ((check_id == 0xF6) && tp_status->status_low.gesture)
  {
    touch->touch_num = 0x00;
    touch->up = 0;
    touch->down = 0;
    touch->gesture = sample_data[6] & 0x07;
    printf("gesture : 0x%02x\n", touch->gesture);
  }
  else
  {
    touch->touch_num = 0x00;
    touch->gesture = 0x00;
  }
  return ESP_OK;
}

esp_err_t read_tp_hdp_status(tp_hdp_status_t *tp_hdp_status)
{
  uint8_t sample_data[8];
  sample_data[0] = 0xFC;
  sample_data[1] = 0x02;
  I2C_Read_Touch(SPD2010_ADDR, (((uint16_t)sample_data[0] << 8) | (sample_data[1])), sample_data, 8);
  tp_hdp_status->status = sample_data[5];
  tp_hdp_status->next_packet_len = (sample_data[2] | sample_data[3] << 8);
  return ESP_OK;
}

esp_err_t Read_HDP_REMAIN_DATA(tp_hdp_status_t *tp_hdp_status)
{
  uint8_t sample_data[32];
  sample_data[0] = 0x00;
  sample_data[1] = 0x03;
  I2C_Read_Touch(SPD2010_ADDR, (((uint16_t)sample_data[0] << 8) | (sample_data[1])), sample_data, tp_hdp_status->next_packet_len);
  return ESP_OK;
}

esp_err_t read_fw_version()
{
  uint8_t sample_data[18];
  uint16_t DVer;
  uint32_t Dummy, PID, ICName_H, ICName_L;
  sample_data[0] = 0x26;
  sample_data[1] = 0x00;
  I2C_Read_Touch(SPD2010_ADDR, (((uint16_t)sample_data[0] << 8) | (sample_data[1])), sample_data, 18);
  Dummy = ((sample_data[0] << 24) | (sample_data[1] << 16) | (sample_data[3] << 8) | (sample_data[0]));
  DVer = ((sample_data[5] << 8) | (sample_data[4]));
  PID = ((sample_data[9] << 24) | (sample_data[8] << 16) | (sample_data[7] << 8) | (sample_data[6]));
  ICName_L = ((sample_data[13] << 24) | (sample_data[12] << 16) | (sample_data[11] << 8) | (sample_data[10])); // "2010"
  ICName_H = ((sample_data[17] << 24) | (sample_data[16] << 16) | (sample_data[15] << 8) | (sample_data[14])); // "SPD"
  printf("Dummy[%d], DVer[%d], PID[%d], Name[%d-%d]\r\n", Dummy, DVer, PID, ICName_H, ICName_L);
  return ESP_OK;
}

esp_err_t tp_read_data(SPD2010_Touch *touch)
{
  tp_status_t tp_status = {0};
  tp_hdp_status_t tp_hdp_status = {0};
  uint32_t loop_counter = 0;
  const uint32_t MAX_LOOPS = 10; // 添加最大循环次数限制，防止无限循环

  read_tp_status_length(&tp_status);

  // 添加调试信息，查看TP状态
  // Serial.printf("DEBUG: TP Status - cpu_run:%d, tic_in_bios:%d, tic_in_cpu:%d, pt_exist:%d, gesture:%d, read_len:%d\n",
  //               tp_status.status_high.cpu_run,
  //               tp_status.status_high.tic_in_bios,
  //               tp_status.status_high.tic_in_cpu,
  //               tp_status.status_low.pt_exist,
  //               tp_status.status_low.gesture,
  //               tp_status.read_len);

  // 检查read_len是否合理，如果不合理则进行修正
  if (tp_status.read_len > 100)
  { // 正常情况下read_len不会超过100
    // Serial.printf("DEBUG: WARNING - Unreasonable read_len(%d), resetting to default value\n", tp_status.read_len);
    tp_status.read_len = 10; // 设置一个合理的默认值
  }
  if (tp_status.status_high.tic_in_bios)
  {
    /* Write Clear TINT Command */
    // Serial.println("DEBUG: tic_in_bios detected, sending clear int cmd");
    write_tp_clear_int_cmd();
    /* Write CPU Start Command */
    // Serial.println("DEBUG: Sending cpu start cmd");
    write_tp_cpu_start_cmd();
  }
  else if (tp_status.status_high.tic_in_cpu)
  {
    /* Write Touch Change Command */
    // Serial.println("DEBUG: tic_in_cpu detected, sending point mode cmd");
    write_tp_point_mode_cmd();
    /* Write Touch Start Command */
    // Serial.println("DEBUG: Sending start cmd");
    write_tp_start_cmd();
    /* Write Clear TINT Command */
    // Serial.println("DEBUG: Sending clear int cmd");
    write_tp_clear_int_cmd();
  }
  else if (tp_status.status_high.cpu_run && tp_status.read_len == 0)
  {
    // Serial.println("DEBUG: cpu_run detected and read_len is 0, sending clear int cmd");
    write_tp_clear_int_cmd();
  }
  else if (tp_status.status_low.pt_exist || tp_status.status_low.gesture)
  {
    /* Read HDP */
    // Serial.println("DEBUG: pt_exist or gesture detected, reading HDP");
    read_tp_hdp(&tp_status, touch);
  hdp_done_check:
    /* Read HDP Status */
    read_tp_hdp_status(&tp_hdp_status);
    if (tp_hdp_status.status == 0x82)
    {
      /* Clear INT */
      // Serial.println("DEBUG: HDP status 0x82 detected, sending clear int cmd");
      write_tp_clear_int_cmd();
    }
    else if (tp_hdp_status.status == 0x00)
    {
      /* Read HDP Remain Data */
      // Serial.println("DEBUG: HDP status 0x00 detected, reading remain data");
      Read_HDP_REMAIN_DATA(&tp_hdp_status);

      // 添加循环计数器检查，防止无限循环
      loop_counter++;
      if (loop_counter < MAX_LOOPS)
      {
        goto hdp_done_check;
      }
      else
      {
        // 超过最大循环次数，跳出循环并清除中断
        write_tp_clear_int_cmd();
        Serial.println("Warning: HDP read loop timed out");
      }
    }
  }
  else if (tp_status.status_high.cpu_run && tp_status.status_low.aux)
  {
    // Serial.println("DEBUG: cpu_run and aux detected, sending clear int cmd");
    write_tp_clear_int_cmd();
  }

  return ESP_OK;
}
