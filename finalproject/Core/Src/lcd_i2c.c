#include "lcd_i2c.h"
#include "i2c.h"

// PCF8574 to HD44780 mapping:
// P7 = D7
// P6 = D6
// P5 = D5
// P4 = D4
// P3 = Backlight
// P2 = EN
// P1 = RW
// P0 = RS

#define RS_BIT 0x01
#define RW_BIT 0x02
#define EN_BIT 0x04
#define BL_BIT 0x08

static void LCD_SendNibble(uint8_t nibble, uint8_t rs){
    uint8_t data = nibble & 0xF0;
    data |= rs;
    data |= BL_BIT; // Backlight on
    
    // Enable Pulse
    uint8_t buffer[4];
    buffer[0] = data | EN_BIT; // EN = 1
    buffer[1] = data | EN_BIT; // EN = 1 (delay)
    buffer[2] = data & ~EN_BIT; // EN = 0
    buffer[3] = data & ~EN_BIT; // EN = 0 (delay)
    
    I2C_WriteBuffer(LCD_ADDR, buffer, 4);
    _sysTick_delay_ms(1);
}

static void LCD_SendByte(uint8_t byte, uint8_t rs){
    uint8_t highNibble = byte & 0xF0;
    uint8_t lowNibble = (byte << 4) & 0xF0;
    
    LCD_SendNibble(highNibble, rs);
    LCD_SendNibble(lowNibble, rs);
}

void LCD_Command(uint8_t cmd){
    LCD_SendByte(cmd, 0); // RS = 0
}

void LCD_Data(uint8_t data){
    LCD_SendByte(data, RS_BIT); // RS = 1
}

void LCD_Clear(void){
    LCD_Command(0x01); // Clear display
    _sysTick_delay_ms(2);
}

void LCD_SetCursor(uint8_t row, uint8_t col){
    uint8_t address;
    switch(row){
        case 0: address = 0x00; break;
        case 1: address = 0x40; break;
        case 2: address = 0x14; break;
        case 3: address = 0x54; break;
        default: address = 0x00;
    }
    address += col;
    LCD_Command(0x80 | address);
}

void LCD_Init(void){
    _sysTick_delay_ms(50); // Wait for power up
    
    // Initialize in 4-bit mode
    LCD_SendNibble(0x30, 0);
    _sysTick_delay_ms(5);
    LCD_SendNibble(0x30, 0);
    _sysTick_delay_ms(1);
    LCD_SendNibble(0x30, 0);
    _sysTick_delay_ms(1);
    LCD_SendNibble(0x20, 0); // Set to 4-bit mode
    _sysTick_delay_ms(1);
    
    // Configure LCD
    LCD_Command(0x28); // 4-bit mode, 2 lines, 5x8 font
    LCD_Command(0x08); // Display off
    LCD_Command(0x01); // Clear display
    _sysTick_delay_ms(2);
    LCD_Command(0x06); // Auto-increment cursor
    LCD_Command(0x0C); // Display on, cursor off
}

void LCD_String(char *str){
    while(*str){
        LCD_Data(*str++);
    }
}
