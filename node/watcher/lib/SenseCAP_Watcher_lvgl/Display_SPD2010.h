#pragma once
#include "TCA9554PWR.h"

// SenseCAP Watcher 背光引脚
#define LCD_Backlight_PIN   8   // GPIO 8
// Backlight   
#define PWM_Channel     1       // PWM Channel   
#define Frequency       20000   // PWM frequency        
#define Resolution      10       // PWM resolution ratio     MAX:13
#define Dutyfactor      500     // PWM Dutyfactor     
#define Backlight_MAX   100

#define EXAMPLE_LCD_WIDTH                   (412)
#define EXAMPLE_LCD_HEIGHT                  (412)
#define EXAMPLE_LCD_COLOR_BITS              (16)

#define ESP_PANEL_HOST_SPI_ID_DEFAULT       (SPI2_HOST)
#define ESP_PANEL_LCD_SPI_MODE              (3)                   // 0/1/2/3, typically set to 0
#define ESP_PANEL_LCD_SPI_CLK_HZ            (40 * 1000 * 1000)    // Should be an integer divisor of 80M, typically set to 40M
#define ESP_PANEL_LCD_SPI_TRANS_QUEUE_SZ    (10)                  // Typically set to 10
#define ESP_PANEL_LCD_SPI_CMD_BITS          (32)                  // Typically set to 32
#define ESP_PANEL_LCD_SPI_PARAM_BITS        (8)                   // Typically set to 8

// SenseCAP Watcher LCD QSPI 引脚
#define ESP_PANEL_LCD_SPI_IO_TE             (-1)  // 暂时不用
#define ESP_PANEL_LCD_SPI_IO_SCK            (7)   // CLK
#define ESP_PANEL_LCD_SPI_IO_DATA0          (9)   // D0
#define ESP_PANEL_LCD_SPI_IO_DATA1          (1)   // D1
#define ESP_PANEL_LCD_SPI_IO_DATA2          (14)  // D2
#define ESP_PANEL_LCD_SPI_IO_DATA3          (13)  // D3
#define ESP_PANEL_LCD_SPI_IO_CS             (45)  // CS
#define EXAMPLE_LCD_PIN_NUM_RST             (-1)
#define EXAMPLE_LCD_PIN_NUM_BK_LIGHT        (-1)

#define EXAMPLE_LCD_BK_LIGHT_ON_LEVEL       (1)
#define EXAMPLE_LCD_BK_LIGHT_OFF_LEVEL !EXAMPLE_LCD_BK_LIGHT_ON_LEVEL

#define ESP_PANEL_HOST_SPI_MAX_TRANSFER_SIZE   (2048)


extern uint8_t LCD_Backlight;

bool SPD2010_Init();

void LCD_Init();
void LCD_addWindow(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend, uint16_t* color);

// backlight
void Backlight_Init();
void Set_Backlight(uint8_t Light);

