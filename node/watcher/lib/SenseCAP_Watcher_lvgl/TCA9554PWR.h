#pragma once

#include <stdio.h>
#include <Wire.h>

/****************************************************** The macro defines the TCA9554PWR information ******************************************************/

#define TCA9554_ADDRESS 0x21 // PCA9535 I2C address (硬件实际是PCA9535，不是TCA9554，地址是0x21)

#define TCA9554_INPUT_REG 0x00    // Input register,input level (PORT0: 0x00, PORT1: 0x01)
#define TCA9554_OUTPUT_REG 0x02   // Output register, high and low level output (PORT0: 0x02, PORT1: 0x03)
#define TCA9554_Polarity_REG 0x04 // The Polarity Inversion register (register 2) allows polarity inversion of pins defined as inputs by the Configuration register.
#define TCA9554_CONFIG_REG 0x06   // Configuration register, mode configuration (PORT0: 0x06, PORT1: 0x07)

#define Low 0
#define High 1
#define EXIO_PIN1 1
#define EXIO_PIN2 2
#define EXIO_PIN3 3
#define EXIO_PIN4 4
#define EXIO_PIN5 5
#define EXIO_PIN6 6
#define EXIO_PIN7 7
#define EXIO_PIN8 8

/*****************************************************  Operation register REG   ****************************************************/
uint8_t I2C_Read_EXIO(uint8_t REG);                          // Read the value of the PCA9535 register REG
uint8_t I2C_Write_EXIO(uint8_t REG, uint8_t Data);           // Write Data to the PCA9535 register REG
uint16_t I2C_Read_EXIO_16(uint8_t start_reg);                // Read 16-bit value from PCA9535 starting at start_reg
uint8_t I2C_Write_EXIO_16(uint8_t start_reg, uint16_t data); // Write 16-bit value to PCA9535

/********************************************************** Set EXIO mode **********************************************************/
void Mode_EXIO(uint8_t Pin, uint8_t State); // Set the mode of the PCA9535 Pin. The default is Output mode (output mode or input mode). State: 0= Output mode 1= input mode
void Mode_EXIOS(uint8_t PinState);          // Set the mode of the 8 pins from PORT0 of PCA9535 with PinState
void Mode_EXIOS_16(uint16_t PinState);      // Set the mode of all 16 pins of PCA9535 with PinState

/********************************************************** Read EXIO status **********************************************************/
uint8_t Read_EXIO(uint8_t Pin);                                // Read the level of the PCA9535 Pin
uint8_t Read_EXIOS(uint8_t REG = TCA9554_INPUT_REG);           // Read the level of all pins of PORT0 of PCA9535
uint16_t Read_EXIOS_16(uint8_t start_reg = TCA9554_INPUT_REG); // Read the level of all 16 pins of PCA9535

/********************************************************** Set the EXIO output status **********************************************************/
void Set_EXIO(uint8_t Pin, uint8_t State); // Sets the level state of the Pin without affecting the other pins
void Set_EXIOS(uint8_t PinState);          // Set 8 pins of PORT0 to the PinState state
void Set_EXIOS_16(uint16_t PinState);      // Set all 16 pins to the PinState state

/********************************************************** Flip EXIO state **********************************************************/
void Set_Toggle(uint8_t Pin); // Flip the level of the PCA9535 Pin

/********************************************************* PCA9535 Initializes the device ***********************************************************/
void TCA9554PWR_Init(uint8_t PinState = 0x00); // Set the 8 pins of PORT0 to PinState state, for example :PinState=0x23, 0010 0011 State (Output mode or input mode) 0= Output mode 1= Input mode. The default value is output mode
void PCA9535_Init(uint16_t PinState = 0x0000); // Set all 16 pins to PinState state, for example :PinState=0x0023, 0000 0000 0010 0011 State (Output mode or input mode) 0= Output mode 1= Input mode. The default value is output mode
