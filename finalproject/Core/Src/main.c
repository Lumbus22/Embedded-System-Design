#include "main.h"
#include "i2c.h"
#include "lcd_i2c.h"
#include "adc.h"

int main(void){
    // Initialize system tick for delays
    SysTick_Init();
    
    // Initialize bare metal peripherals
    I2C1_Init();
    LCD_Init();
    ADC1_Init();
    
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String("EGC433 Final");
    LCD_SetCursor(1, 0);
    LCD_String("Init Complete");
    _sysTick_delay_ms(2000);
    
    char buffer[20];
    
    while(1){
        // Read Temperature from LM35 on PA4
        float currentTemp = LM35_GetTemp();
        
        // Format string
        sprintf(buffer, "Temp: %.1f C   ", currentTemp);
        
        // Display on LCD
        LCD_SetCursor(0, 0);
        LCD_String("HVAC System");
        LCD_SetCursor(1, 0);
        LCD_String(buffer);
        
        // Update every 500ms
        _sysTick_delay_ms(500);
    }
}
