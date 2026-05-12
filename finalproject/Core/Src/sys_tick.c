#include "main.h"

void SysTick_Init(void){
    SysTick->LOAD = (SystemCoreClock / 1000) - 1; // Assuming SystemCoreClock is set, or default 16MHz
    SysTick->VAL = 0;
    SysTick->CTRL &= ~(0x07);
    SysTick->CTRL |= 0x05; // Processor clock, Enable
}

void _sysTick_delay_ms(int n){
    int i;
    for(i = 0; i < n; i++){
        while((SysTick->CTRL & 0x10000) == 0){}
    }
}
