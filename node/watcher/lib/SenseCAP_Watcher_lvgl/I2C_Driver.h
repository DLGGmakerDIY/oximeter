#pragma once
#include <Wire.h>
#include <mutex>

#define I2C_MASTER_FREQ_HZ (400000) /*!< I2C master clock frequency */

// 声明I2C互斥锁，用于保护I2C总线的并发访问
extern std::mutex i2c1_mutex;
extern std::mutex i2c0_mutex;

// I2C0 - IO扩展芯片 (PCA9535)
#define I2C0_SCL_PIN 48 // GPIO 48
#define I2C0_SDA_PIN 47 // GPIO 47

// I2C1 - 触摸屏
#define I2C1_SDA_PIN 39 // GPIO 39
#define I2C1_SCL_PIN 38 // GPIO 38

// 注意：Wire1对象由ESP32 Arduino框架预定义，无需额外声明

void I2C_Init(void);
void I2C1_Init(void);

// 8位寄存器地址的I2C读写函数
bool I2C_Read(uint8_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length);
bool I2C_Write(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length);
bool I2C1_Read(uint8_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length);
bool I2C1_Write(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length);

// 16位寄存器地址的I2C读写函数（用于触摸控制器等设备）
bool I2C_Read16(uint8_t Driver_addr, uint16_t Reg_addr, uint8_t *Reg_data, uint32_t Length);
bool I2C_Write16(uint8_t Driver_addr, uint16_t Reg_addr, const uint8_t *Reg_data, uint32_t Length);
bool I2C1_Read16(uint8_t Driver_addr, uint16_t Reg_addr, uint8_t *Reg_data, uint32_t Length);
bool I2C1_Write16(uint8_t Driver_addr, uint16_t Reg_addr, const uint8_t *Reg_data, uint32_t Length);
