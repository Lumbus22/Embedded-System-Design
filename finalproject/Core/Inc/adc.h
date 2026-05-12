#ifndef ADC_H
#define ADC_H

#include "main.h"

void ADC1_Init(void);
uint16_t ADC1_Read(void);
float LM35_GetTemp(void);

#endif // ADC_H
