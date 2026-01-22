#include "I2C_Driver.h"
#include <mutex>

// I2C总线互斥锁，用于保护I2C总线的并发访问
std::mutex i2c1_mutex;
std::mutex i2c0_mutex;

void I2C_Init(void)
{
    // 初始化I2C0（IO扩展芯片）
    Wire.begin(I2C0_SDA_PIN, I2C0_SCL_PIN);
    Wire.setClock(I2C_MASTER_FREQ_HZ);
}

void I2C1_Init(void)
{
    // 初始化I2C1（触摸屏）
    Wire1.begin(I2C1_SDA_PIN, I2C1_SCL_PIN);
    Wire1.setClock(I2C_MASTER_FREQ_HZ);
}

bool I2C_Read(uint8_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
    // 使用互斥锁保护I2C0访问
    std::lock_guard<std::mutex> lock(i2c0_mutex);

    // 使用I2C0读取
    Wire.beginTransmission(Driver_addr);
    Wire.write(Reg_addr);
    if (Wire.endTransmission(true))
    {
        printf("The I2C0 transmission fails. - I2C Read\r\n");
        return false;
    }
    Wire.requestFrom(Driver_addr, Length);
    for (int i = 0; i < Length; i++)
    {
        *Reg_data++ = Wire.read();
    }
    return true;
}

bool I2C_Write(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
    // 使用互斥锁保护I2C0访问
    std::lock_guard<std::mutex> lock(i2c0_mutex);

    // 使用I2C0写入
    Wire.beginTransmission(Driver_addr);
    Wire.write(Reg_addr);
    for (int i = 0; i < Length; i++)
    {
        Wire.write(*Reg_data++);
    }
    if (Wire.endTransmission(true))
    {
        printf("The I2C0 transmission fails. - I2C Write\r\n");
        return false;
    }
    return true;
}

bool I2C1_Read(uint8_t Driver_addr, uint8_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
    // 使用互斥锁保护I2C1访问
    std::lock_guard<std::mutex> lock(i2c1_mutex);

    // 使用I2C1读取
    Wire1.beginTransmission(Driver_addr);
    Wire1.write(Reg_addr);
    if (Wire1.endTransmission(true))
    {
        printf("The I2C1 transmission fails. - I2C Read\r\n");
        return false;
    }
    Wire1.requestFrom(Driver_addr, Length);
    for (int i = 0; i < Length; i++)
    {
        *Reg_data++ = Wire1.read();
    }
    return true;
}

bool I2C1_Write(uint8_t Driver_addr, uint8_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
    // 使用互斥锁保护I2C1访问
    std::lock_guard<std::mutex> lock(i2c1_mutex);

    // 使用I2C1写入
    Wire1.beginTransmission(Driver_addr);
    Wire1.write(Reg_addr);
    for (int i = 0; i < Length; i++)
    {
        Wire1.write(*Reg_data++);
    }
    if (Wire1.endTransmission(true))
    {
        printf("The I2C1 transmission fails. - I2C Write\r\n");
        return false;
    }
    return true;
}

// 16位寄存器地址的I2C读写函数实现
bool I2C_Read16(uint8_t Driver_addr, uint16_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
    // 使用I2C0读取16位寄存器地址
    Wire.beginTransmission(Driver_addr);
    Wire.write((uint8_t)(Reg_addr >> 8));   // 高字节
    Wire.write((uint8_t)(Reg_addr & 0xFF)); // 低字节
    if (Wire.endTransmission(true))
    {
        printf("The I2C0 transmission fails. - I2C Read16\r\n");
        return false;
    }
    Wire.requestFrom(Driver_addr, Length);
    for (int i = 0; i < Length; i++)
    {
        *Reg_data++ = Wire.read();
    }
    return true;
}

bool I2C_Write16(uint8_t Driver_addr, uint16_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
    // 使用I2C0写入16位寄存器地址
    Wire.beginTransmission(Driver_addr);
    Wire.write((uint8_t)(Reg_addr >> 8));   // 高字节
    Wire.write((uint8_t)(Reg_addr & 0xFF)); // 低字节
    for (int i = 0; i < Length; i++)
    {
        Wire.write(*Reg_data++);
    }
    if (Wire.endTransmission(true))
    {
        printf("The I2C0 transmission fails. - I2C Write16\r\n");
        return false;
    }
    return true;
}

bool I2C1_Read16(uint8_t Driver_addr, uint16_t Reg_addr, uint8_t *Reg_data, uint32_t Length)
{
    // 使用互斥锁保护I2C1访问
    std::lock_guard<std::mutex> lock(i2c1_mutex);

    // 使用I2C1读取16位寄存器地址
    Wire1.beginTransmission(Driver_addr);
    Wire1.write((uint8_t)(Reg_addr >> 8));   // 高字节
    Wire1.write((uint8_t)(Reg_addr & 0xFF)); // 低字节
    if (Wire1.endTransmission(true))
    {
        printf("The I2C1 transmission fails. - I2C Read16\r\n");
        return false;
    }
    Wire1.requestFrom(Driver_addr, Length);
    for (int i = 0; i < Length; i++)
    {
        *Reg_data++ = Wire1.read();
    }
    return true;
}

bool I2C1_Write16(uint8_t Driver_addr, uint16_t Reg_addr, const uint8_t *Reg_data, uint32_t Length)
{
    // 使用互斥锁保护I2C1访问
    std::lock_guard<std::mutex> lock(i2c1_mutex);

    // 使用I2C1写入16位寄存器地址
    Wire1.beginTransmission(Driver_addr);
    Wire1.write((uint8_t)(Reg_addr >> 8));   // 高字节
    Wire1.write((uint8_t)(Reg_addr & 0xFF)); // 低字节
    for (int i = 0; i < Length; i++)
    {
        Wire1.write(*Reg_data++);
    }
    if (Wire1.endTransmission(true))
    {
        printf("The I2C1 transmission fails. - I2C Write16\r\n");
        return false;
    }
    return true;
}
