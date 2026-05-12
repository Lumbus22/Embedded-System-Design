#include "adc.h"

// LM35 connected to PA4 (ADC12_IN4)
void ADC1_Init(void){
    // 1. Enable GPIOA clock
    RCC->AHB1ENR |= (1 << 0); // GPIOAEN

    // 2. Set PA4 to Analog mode (MODER = 11)
    GPIOA->MODER |= (3 << 8); // PA4 analog mode

    // 3. Enable ADC1 clock
    RCC->APB2ENR |= (1 << 8); // ADC1EN

    // 4. Configure ADC1 parameters
    ADC1->CR1 &= ~(3 << 24); // 12-bit resolution (00)
    
    // 5. Set channel sequence
    ADC1->SQR3 = 4; // 1st conversion in regular sequence is CH4
    ADC1->SQR1 = 0; // 1 conversion

    // 6. Enable ADC
    ADC1->CR2 |= (1 << 0); // ADON = 1
    
    // Small delay for ADC to power up
    _sysTick_delay_ms(1);
}

uint16_t ADC1_Read(void){
    // 1. Start conversion
    ADC1->CR2 |= (1 << 30); // SWSTART
    
    // 2. Wait for End Of Conversion (EOC)
    while (!(ADC1->SR & (1 << 1)));
    
    // 3. Read and return Data Register
    return ADC1->DR;
}

float LM35_GetTemp(void){
    uint16_t adc_val = ADC1_Read();
    
    // LM35 provides 10mV per degree Celsius.
    // Assuming 3.3V reference voltage and 12-bit ADC (4096 steps).
    // Voltage in mV = (adc_val * 3300.0f) / 4096.0f;
    // Temp in C = Voltage_mV / 10.0f;
    
    float temp_c = (adc_val * 330.0f) / 4096.0f;
    return temp_c;
}
