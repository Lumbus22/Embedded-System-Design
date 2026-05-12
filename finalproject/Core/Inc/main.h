#ifndef MAIN_H
#define MAIN_H

#include "stm32f446xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

void SysTick_Init(void);
void _sysTick_delay_ms(int n);

#endif // MAIN_H
