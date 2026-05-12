#include "i2c.h"

// I2C1 SCL on PB8, SDA on PB9
void I2C1_Init(void){
    // 1. Enable GPIOB clock and I2C1 clock
    RCC->AHB1ENR |= (1 << 1);  // GPIOBEN
    RCC->APB1ENR |= (1 << 21); // I2C1EN

    // 2. Configure PB8 and PB9 as Alternate Function
    GPIOB->MODER &= ~((3 << 16) | (3 << 18));
    GPIOB->MODER |=  ((2 << 16) | (2 << 18)); // AF mode

    // 3. Configure PB8 and PB9 as Open Drain
    GPIOB->OTYPER |= ((1 << 8) | (1 << 9));

    // 4. Configure PB8 and PB9 with Pull-up resistors
    GPIOB->PUPDR &= ~((3 << 16) | (3 << 18));
    GPIOB->PUPDR |=  ((1 << 16) | (1 << 18));

    // 5. Configure PB8 and PB9 High Speed
    GPIOB->OSPEEDR |= ((3 << 16) | (3 << 18));

    // 6. Set Alternate Function AF4 for PB8 and PB9
    GPIOB->AFR[1] &= ~((0xF << 0) | (0xF << 4)); // Clear AFRH for PB8, PB9
    GPIOB->AFR[1] |=  ((4 << 0) | (4 << 4));     // Set AF4

    // 7. I2C1 Reset
    I2C1->CR1 |= (1 << 15);
    I2C1->CR1 &= ~(1 << 15);

    // 8. I2C1 Clock Control
    // Default system clock for Nucleo without special init is 16MHz (HSI)
    // APB1 is 16MHz. Peripheral clock = 16MHz
    I2C1->CR2 = 16; 

    // Standard mode 100kHz: CCR = PCLK1 / (2 * 100kHz) = 16MHz / 200kHz = 80
    I2C1->CCR = 80;

    // Maximum rise time: TRISE = (Trise_max / Tpclk1) + 1 = (1000ns / 62.5ns) + 1 = 17
    I2C1->TRISE = 17;

    // 9. Enable I2C1
    I2C1->CR1 |= (1 << 0); // PE = 1
}

void I2C1_Start(void){
    I2C1->CR1 |= (1 << 8); // START
    while (!(I2C1->SR1 & (1 << 0))); // Wait for SB (Start Bit)
}

void I2C1_Address(uint8_t address){
    I2C1->DR = address; // Send address (should already include R/W bit)
    while (!(I2C1->SR1 & (1 << 1))); // Wait for ADDR (Address matched)
    uint32_t temp = I2C1->SR1; // Read SR1 and SR2 to clear ADDR bit
    temp = I2C1->SR2;
    (void)temp;
}

void I2C1_Write(uint8_t data){
    while (!(I2C1->SR1 & (1 << 7))); // Wait for TXE (Data register empty)
    I2C1->DR = data;
    while (!(I2C1->SR1 & (1 << 2))); // Wait for BTF (Byte transfer finished)
}

void I2C1_Stop(void){
    I2C1->CR1 |= (1 << 9); // STOP
}

void I2C_WriteBuffer(uint8_t addr, uint8_t *data, uint8_t len){
    I2C1_Start();
    I2C1_Address(addr);
    for (uint8_t i = 0; i < len; i++){
        I2C1_Write(data[i]);
    }
    I2C1_Stop();
}
