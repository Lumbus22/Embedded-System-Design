#ifndef LCD_I2C_H
#define LCD_I2C_H

#include "main.h"

// Define I2C address for LCD backpack (usually 0x27 or 0x3F, shifted left by 1 for HAL/bare-metal 8-bit addr)
// 0x27 << 1 = 0x4E
#define LCD_ADDR 0x4E

void LCD_Init(void);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t data);
void LCD_String(char *str);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t col);

#endif // LCD_I2C_H
