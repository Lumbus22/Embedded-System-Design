/***************************************************
* EGC433 - Assignment 6
* Part 2 - Interrupt Based System
*
* PA5  = On-board LED  -- toggled by TIM2 update interrupt every 3000ms
* PC9  = Off-board LED -- controlled by EXTI13 (PC13 rising/falling edge)
* PC13 = On-board push button -- triggers EXTI15_10 on both edges
*
* Interrupt sources:
*   TIM2_IRQn       -- fires every 3000ms, toggles PA5
*   EXTI15_10_IRQn  -- fires on PC13 edge, mirrors button state to PC9
*
* The main loop is intentionally empty; all work happens in ISRs.
* This eliminates the missed-button problem seen in Part 1.
*
* Author: Columbus Enslen
* Date Last Modified: 3/15/2026
***************************************************/

#include "main.h"
#include <stdbool.h>

/* ---- Function Prototypes ---- */
void SysTick_Init(void);
void _sysTick_delay_ms(int n);

void initPA5(void);
void initPC9(void);
void initPC13_EXTI(void);
void initTIM2_Interrupt(void);


/* ============================================================
 *  MAIN -- initialise peripherals then idle forever
 * ============================================================ */
int main(void)
{
    SysTick_Init();

    initPA5();             /* On-board LED output                     */
    initPC9();             /* Off-board LED output                    */
    initTIM2_Interrupt();  /* 3-second timer interrupt for PA5 toggle */
    initPC13_EXTI();       /* Button edge interrupt for PC9 control   */


    while (1) { } //The CPU has nothing to do
}


/* ============================================================
 *  TIM2 ISR -- fires every 3000 ms, toggles PA5
 * ============================================================ */
void TIM2_IRQHandler(void)
{
    if (TIM2->SR & (1 << 0))          /* Update-interrupt flag (UIF)     */
    {
        TIM2->SR &= ~(1 << 0);        /* Clear UIF before returning      */
        GPIOA->ODR ^= (1 << 5);       /* Toggle PA5                      */
    }
}


/* ============================================================
 *  EXTI15_10 ISR -- fires on PC13 rising OR falling edge
 * ============================================================ */
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR & (1 << 13))         /* PC13 pending bit                */
    {
        EXTI->PR |= (1 << 13);        /* Clear pending bit (write 1)     */

        /* Read current button state and drive PC9 accordingly.
           PC13 is active-low: pin LOW  = button pressed.              */
        if (!(GPIOC->IDR & (1 << 13)))
            GPIOC->ODR |=  (1 << 9);  /* Button pressed  -> LED on       */
        else
            GPIOC->ODR &= ~(1 << 9);  /* Button released -> LED off      */
    }
}


/* ============================================================
 *  TIM2 -- 3000 ms update interrupt
 * ============================================================ */
void initTIM2_Interrupt(void)
{
    RCC->APB1ENR |= (1 << 0);         /* Enable TIM2 peripheral clock    */

    /* Prescaler: 16 MHz / 16 000 = 1 kHz  ->  1 ms per tick            */
    TIM2->PSC  = 15999;

    /* Auto-reload: count 0..2999 = 3000 ticks = 3000 ms                */
    TIM2->ARR  = 2999;

    TIM2->CNT  = 0;                   /* Start count from zero           */
    TIM2->EGR |= (1 << 0);           /* Force register update (load PSC) */
    TIM2->SR  &= ~(1 << 0);          /* Clear any pending update flag    */

    TIM2->DIER |= (1 << 0);          /* Enable update interrupt (UIE)    */
    TIM2->CR1  |= (1 << 0);          /* Start timer (CEN)               */

    NVIC_EnableIRQ(TIM2_IRQn);       /* Unmask TIM2 in the NVIC          */
}


/* ============================================================
 *  PC13 EXTI -- rising + falling edge interrupt
 * ============================================================ */
void initPC13_EXTI(void)
{
    /* --- GPIO: PC13 as input (default MODER = 00) --- */
    RCC->AHB1ENR  |=  (1 << 2);      /* Enable GPIOC clock              */
    GPIOC->MODER  &= ~(3U << 26);    /* PC13 = input                    */

    /* --- SYSCFG: route EXTI13 to Port C --- */
    RCC->APB2ENR  |=  (1 << 14);     /* Enable SYSCFG clock             */

    /* EXTICR[3] selects the source port for EXTI lines 12-15.
       EXTI13 uses bits [7:4]; 0b0010 = Port C.                        */
    SYSCFG->EXTICR[3] &= ~(0xF << 4);
    SYSCFG->EXTICR[3] |=  (0x2 << 4);

    /* --- EXTI: unmask line 13 and enable both edges --- */
    EXTI->IMR  |= (1 << 13);         /* Unmask interrupt on line 13     */
    EXTI->RTSR |= (1 << 13);         /* Rising  edge trigger (release)  */
    EXTI->FTSR |= (1 << 13);         /* Falling edge trigger (press)    */

    NVIC_EnableIRQ(EXTI15_10_IRQn);  /* Lines 10-15 share one IRQ       */
}


/* ============================================================
 *  SysTick -- 1 ms tick source
 * ============================================================ */
void SysTick_Init(void)
{
    SysTick->LOAD = (SystemCoreClock / 1000) - 1;
    SysTick->VAL  = 0;
    SysTick->CTRL &= ~(0x07);
    SysTick->CTRL |=  0x05;
}

void _sysTick_delay_ms(int n)
{
    int i;
    for (i = 0; i < n; i++)
    {
        while ((SysTick->CTRL & 0x10000) == 0) {}
    }
}


/* ============================================================
 *  PA5 -- On-board LED
 * ============================================================ */
void initPA5(void)
{
    RCC->AHB1ENR |= (1 << 0);
    GPIOA->MODER &= ~(3U << 10);
    GPIOA->MODER |=  (1  << 10);     /* PA5 = output                    */
    GPIOA->ODR   &= ~(1  <<  5);     /* Start with LED off              */
}


/* ============================================================
 *  PC9 -- Off-board LED
 * ============================================================ */
void initPC9(void)
{
    RCC->AHB1ENR |=  (1 << 2);
    GPIOC->MODER &= ~(3U << 18);
    GPIOC->MODER |=  (1  << 18);     /* PC9 = output                    */
    GPIOC->ODR   &= ~(1  <<  9);     /* Start with LED off              */
}
