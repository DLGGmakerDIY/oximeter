#include "TCA9554PWR.h"
#include "I2C_Driver.h"

/*****************************************************  Operation register REG   ****************************************************/
// 读取寄存器值 - 支持16位读取
uint8_t I2C_Read_EXIO(uint8_t REG) // Read the value of the PCA9535 register REG
{
  uint8_t bitsStatus;

  // 直接调用Wire库函数进行I2C通信，不使用I2C_Read函数避免死锁
  // 使用I2C0互斥锁保护总线访问
  std::lock_guard<std::mutex> lock(i2c0_mutex);

  Wire.beginTransmission(TCA9554_ADDRESS);
  Wire.write(REG);
  if (Wire.endTransmission(true))
  {
    printf("The I2C transmission fails. - I2C Read EXIO\r\n");
    return 0;
  }
  Wire.requestFrom(TCA9554_ADDRESS, 1);
  if (Wire.available() == 1)
  {
    bitsStatus = Wire.read();
    return bitsStatus;
  }
  else
  {
    printf("The I2C transmission fails. - I2C Read EXIO\r\n");
    return 0;
  }
}

// 写入寄存器值 - 支持16位写入
uint8_t I2C_Write_EXIO(uint8_t REG, uint8_t Data) // Write Data to the PCA9535 register REG
{
  // 直接调用Wire库函数进行I2C通信，不使用I2C_Write函数避免死锁
  // 使用I2C0互斥锁保护总线访问
  std::lock_guard<std::mutex> lock(i2c0_mutex);

  Wire.beginTransmission(TCA9554_ADDRESS);
  Wire.write(REG);
  Wire.write(Data);
  if (Wire.endTransmission(true))
  {
    printf("The I2C transmission fails. - I2C Write EXIO\r\n");
    return -1;
  }
  return 0;
}

// 读取16位寄存器值（用于读取完整的16位IO状态）
uint16_t I2C_Read_EXIO_16(uint8_t start_reg)
{
  uint8_t data[2];

  // 直接调用Wire库函数进行I2C通信，不使用I2C_Read函数避免死锁
  // 使用I2C0互斥锁保护总线访问
  std::lock_guard<std::mutex> lock(i2c0_mutex);

  Wire.beginTransmission(TCA9554_ADDRESS);
  Wire.write(start_reg);
  if (Wire.endTransmission(true))
  {
    printf("The I2C transmission fails. - I2C Read EXIO 16\r\n");
    return 0;
  }
  Wire.requestFrom(TCA9554_ADDRESS, 2);
  if (Wire.available() == 2)
  {
    data[0] = Wire.read();
    data[1] = Wire.read();
    return (data[1] << 8) | data[0]; // PCA9535是小端序，先读取低字节（PORT0），再读取高字节（PORT1）
  }
  else
  {
    printf("The I2C transmission fails. - I2C Read EXIO 16\r\n");
    return 0;
  }
}

// 写入16位寄存器值（用于设置完整的16位IO状态）
uint8_t I2C_Write_EXIO_16(uint8_t start_reg, uint16_t data)
{
  uint8_t write_data[2];
  write_data[0] = data & 0xFF;        // PORT0数据
  write_data[1] = (data >> 8) & 0xFF; // PORT1数据

  // 直接调用Wire库函数进行I2C通信，不使用I2C_Write函数避免死锁
  // 使用I2C0互斥锁保护总线访问
  std::lock_guard<std::mutex> lock(i2c0_mutex);

  Wire.beginTransmission(TCA9554_ADDRESS);
  Wire.write(start_reg);
  Wire.write(write_data[0]);
  Wire.write(write_data[1]);
  if (Wire.endTransmission(true))
  {
    printf("The I2C transmission fails. - I2C Write EXIO 16\r\n");
    return -1;
  }
  return 0;
}

/********************************************************** Set EXIO mode **********************************************************/
void Mode_EXIO(uint8_t Pin, uint8_t State) // Set the mode of the PCA9535 Pin. The default is Output mode (output mode or input mode). State: 0= Output mode 1= input mode
{
  if (Pin < 17 && Pin > 0)
  {
    if (Pin <= 8) // PORT0 (P0.0-P0.7) - 引脚1-8
    {
      uint8_t bitsStatus = I2C_Read_EXIO(TCA9554_CONFIG_REG);
      uint8_t Data;
      if (State == 1)
        Data = (0x01 << (Pin - 1)) | bitsStatus;
      else if (State == 0)
        Data = (~(0x01 << (Pin - 1))) & bitsStatus;
      uint8_t result = I2C_Write_EXIO(TCA9554_CONFIG_REG, Data);
      if (result != 0)
      {
        printf("I/O Configuration Failure !!!\r\n");
      }
    }
    else // PORT1 (P1.0-P1.7) - 引脚9-16
    {
      uint8_t bitsStatus = I2C_Read_EXIO(TCA9554_CONFIG_REG + 1); // PORT1的配置寄存器地址是PORT0配置寄存器地址+1
      uint8_t Data;
      if (State == 1)
        Data = (0x01 << (Pin - 9)) | bitsStatus;
      else if (State == 0)
        Data = (~(0x01 << (Pin - 9))) & bitsStatus;
      uint8_t result = I2C_Write_EXIO(TCA9554_CONFIG_REG + 1, Data);
      if (result != 0)
      {
        printf("I/O Configuration Failure !!!\r\n");
      }
    }
  }
  else
    printf("Parameter error, please enter the correct parameter!\r\n");
}

void Mode_EXIOS(uint8_t PinState) // Set the mode of the 7 pins from the TCA9554PWR with PinState
{
  uint8_t result = I2C_Write_EXIO(TCA9554_CONFIG_REG, PinState);
  if (result != 0)
  {
    printf("I/O Configuration Failure !!!\r\n");
  }
}

/********************************************************** Read EXIO status **********************************************************/
uint8_t Read_EXIO(uint8_t Pin) // Read the level of the PCA9535 Pin
{
  if (Pin < 17 && Pin > 0)
  {
    if (Pin <= 8) // PORT0 (P0.0-P0.7) - 引脚1-8
    {
      uint8_t inputBits = I2C_Read_EXIO(TCA9554_INPUT_REG);
      uint8_t bitStatus = (inputBits >> (Pin - 1)) & 0x01;
      return bitStatus;
    }
    else // PORT1 (P1.0-P1.7) - 引脚9-16
    {
      uint8_t inputBits = I2C_Read_EXIO(TCA9554_INPUT_REG + 1); // PORT1的输入寄存器地址是PORT0输入寄存器地址+1
      uint8_t bitStatus = (inputBits >> (Pin - 9)) & 0x01;
      return bitStatus;
    }
  }
  else
  {
    printf("Parameter error, please enter the correct parameter!\r\n");
    return 0;
  }
}

uint8_t Read_EXIOS(uint8_t REG) // Read the level of all pins of PCA9535 PORT0, the default read input level state, want to get the current IO output state, pass the parameter TCA9554_OUTPUT_REG, such as Read_EXIOS(TCA9554_OUTPUT_REG);
{
  uint8_t inputBits = I2C_Read_EXIO(REG);
  return inputBits;
}

/********************************************************** Set the EXIO output status **********************************************************/
void Set_EXIO(uint8_t Pin, uint8_t State) // Sets the level state of the Pin without affecting the other pins
{
  if (State < 2 && Pin < 17 && Pin > 0)
  {
    if (Pin <= 8) // PORT0 (P0.0-P0.7) - 引脚1-8
    {
      uint8_t bitsStatus = Read_EXIOS(TCA9554_OUTPUT_REG);
      uint8_t Data;
      if (State == 1)
        Data = (0x01 << (Pin - 1)) | bitsStatus;
      else if (State == 0)
        Data = (~(0x01 << (Pin - 1))) & bitsStatus;
      uint8_t result = I2C_Write_EXIO(TCA9554_OUTPUT_REG, Data);
      if (result != 0)
      {
        printf("Failed to set GPIO!!!\r\n");
      }
    }
    else // PORT1 (P1.0-P1.7) - 引脚9-16
    {
      uint8_t bitsStatus = Read_EXIOS(TCA9554_OUTPUT_REG + 1); // PORT1的输出寄存器地址是PORT0输出寄存器地址+1
      uint8_t Data;
      if (State == 1)
        Data = (0x01 << (Pin - 9)) | bitsStatus;
      else if (State == 0)
        Data = (~(0x01 << (Pin - 9))) & bitsStatus;
      uint8_t result = I2C_Write_EXIO(TCA9554_OUTPUT_REG + 1, Data);
      if (result != 0)
      {
        printf("Failed to set GPIO!!!\r\n");
      }
    }
  }
  else
    printf("Parameter error, please enter the correct parameter!\r\n");
}

void Set_EXIOS(uint8_t PinState) // Set 7 pins to the PinState state such as :PinState=0x23, 0010 0011 state (the highest bit is not used)
{
  uint8_t result = I2C_Write_EXIO(TCA9554_OUTPUT_REG, PinState);
  if (result != 0)
  {
    printf("Failed to set GPIO!!!\r\n");
  }
}

/********************************************************** Flip EXIO state **********************************************************/
void Set_Toggle(uint8_t Pin) // Flip the level of the TCA9554PWR Pin
{
  uint8_t bitsStatus = Read_EXIO(Pin);
  Set_EXIO(Pin, (bool)!bitsStatus);
}

/********************************************************* PCA9535 16位IO操作函数 ***********************************************************/

// 设置16位IO模式（所有16个引脚）
void Mode_EXIOS_16(uint16_t PinState) // Set the mode of all 16 pins of PCA9535 with PinState
{
  uint8_t result = I2C_Write_EXIO_16(TCA9554_CONFIG_REG, PinState);
  if (result != 0)
  {
    printf("I/O Configuration Failure !!!\r\n");
  }
}

// 读取16位IO状态（所有16个引脚）
uint16_t Read_EXIOS_16(uint8_t start_reg) // Read the level of all 16 pins of PCA9535
{
  uint8_t data[2];

  // 使用I2C0互斥锁保护总线访问
  std::lock_guard<std::mutex> lock(i2c0_mutex);

  if (I2C_Read(TCA9554_ADDRESS, start_reg, data, 2))
  {
    return (data[1] << 8) | data[0]; // PCA9535是小端序，先读取低字节（PORT0），再读取高字节（PORT1）
  }
  else
  {
    printf("The I2C transmission fails. - Read EXIOS 16\r\n");
    return 0;
  }
}

// 设置16位IO输出状态（所有16个引脚）
void Set_EXIOS_16(uint16_t PinState) // Set all 16 pins to the PinState state
{
  uint8_t result = I2C_Write_EXIO_16(TCA9554_OUTPUT_REG, PinState);
  if (result != 0)
  {
    printf("Failed to set GPIO!!!\r\n");
  }
}

/********************************************************* TCA9554PWR Initializes the device ***********************************************************/
void TCA9554PWR_Init(uint8_t PinState) // Set the 8 pins of PORT0 to PinState state, for example :PinState=0x23, 0010 0011 State  (Output mode or input mode) 0= Output mode 1= Input mode. The default value is output mode
{
  Mode_EXIOS(PinState);
}

/********************************************************* PCA9535 Initializes the device ***********************************************************/
void PCA9535_Init(uint16_t PinState) // Set all 16 pins to PinState state, for example :PinState=0x0023, 0000 0000 0010 0011 State  (Output mode or input mode) 0= Output mode 1= Input mode. The default value is output mode
{
  Mode_EXIOS_16(PinState);
}
