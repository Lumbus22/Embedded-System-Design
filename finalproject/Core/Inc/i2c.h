#ifndef I2C_H
#define I2C_H

#include "main.h"

void I2C1_Init(void);
void I2C1_Start(void);
void I2C1_Write(uint8_t data);
void I2C1_Address(uint8_t address);
void I2C1_Stop(void);
void I2C_WriteBuffer(uint8_t addr, uint8_t *data, uint8_t len);

#endif // I2C_H
