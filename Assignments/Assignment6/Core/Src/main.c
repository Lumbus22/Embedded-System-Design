/***************************************************
* Interrupts / Embedded_Assignment_6
* Part 3 - USART2 Receive Interrupt + LCD Display
*
* USART2 pins (AF7):
*   PA2 = USART2_TX
*   PA3 = USART2_RX
*   Baud: 9600, 8N1
*
* LCD (8-bit parallel, HD44780):
*   PB5 = RS   PB6 = RW   PB7 = EN
*   PC0-PC7 = D0-D7 (8-bit data bus)
*
* Behaviour:
*   - Each received byte is appended to rxBuffer[]
*   - When '\r' is received, readyFlag is set
*   - Main loop clears LCD and prints buffer (up to 2 lines)
*   - Buffer resets after each print
*
* Author: Columbus Enslen
* Date Last Modified: 3/15/2026
*
***************************************************/


#include "main.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 33    //32 visible chars + null terminator

//Buffer and flag shared between ISR and main
volatile char    rxBuffer[BUFFER_SIZE];
volatile uint8_t bufferIndex = 0;
volatile bool    readyFlag   = false;

//Timer init
void TIM1_init(void);

//Delays
void SysTick_Init(void);
void _sysTick_delay_ms(int n);
void _delay_us(int n);

//USART
void USART2_init(void);

//LCD
void LCD_Init_8Bit(void);
void LCD_Pulse_EN(void);
void LCD_Command(unsigned char cmd);
void LCD_Data(unsigned char data);
void LCD_String(char* str);
void LCD_PrintBuffer(void);


int main(void)
{
	TIM1_init();
	SysTick_Init();
	LCD_Init_8Bit();
	USART2_init();     //Enable USART2 with RXNE interrupt

	while(1){
		if(readyFlag){
			readyFlag = false;           //Acknowledge the flag

			LCD_Command(0x01);           //Clear display
			_sysTick_delay_ms(2);        //Clear needs extra settle time

			LCD_PrintBuffer();           //Write buffer to LCD (1 or 2 lines)

			//Reset buffer for next incoming string
			bufferIndex = 0;
			rxBuffer[0] = '\0';
		}
	}
}


//USART2 ISR -- fires on every received byte (RXNE)
void USART2_IRQHandler(void){
	if(USART2 -> SR & (1 << 5)){             //RXNE: receive register not empty
		char received = (char)(USART2 -> DR & 0xFF); //Reading DR clears RXNE

		if(received == '\r'){
			//Carriage return = end of message, null-terminate and raise flag
			rxBuffer[bufferIndex] = '\0';
			readyFlag = true;
		}
		else if(bufferIndex < (BUFFER_SIZE - 1)){
			rxBuffer[bufferIndex++] = received; //Append byte if buffer has room
		}
		//If buffer is full, additional bytes are dropped until '\r'
	}
}


//Print rxBuffer across up to 2 lines (16 chars each)
void LCD_PrintBuffer(void){
	char local[BUFFER_SIZE];
	strncpy(local, (const char*)rxBuffer, BUFFER_SIZE);
	local[BUFFER_SIZE - 1] = '\0';

	int len = (int)strlen(local);

	//--- Line 1 (DDRAM 0x00, up to 16 chars) ---
	LCD_Command(0x80 | 0x00);               //Move cursor to start of line 1
	_sysTick_delay_ms(2);

	if(len <= 16){
		LCD_String(local);                   //Fits on one line
	}
	else{
		//Print first 16 chars on line 1
		char line1[17];
		strncpy(line1, local, 16);
		line1[16] = '\0';
		LCD_String(line1);

		//--- Line 2 (DDRAM 0x40, up to 16 chars) ---
		LCD_Command(0x80 | 0x40);           //Move cursor to start of line 2
		_sysTick_delay_ms(2);

		char line2[17];
		strncpy(line2, local + 16, 16);     //Copy remaining chars (max 16)
		line2[16] = '\0';
		LCD_String(line2);
	}
}


void USART2_init(void){
	RCC -> AHB1ENR |= 1 << 0;               //Enable GPIOA clock
	RCC -> APB1ENR |= 1 << 17;              //Enable USART2 clock

	//PA2 (TX) and PA3 (RX) as Alternate Function
	GPIOA -> MODER &= ~((3 << 4) | (3 << 6));
	GPIOA -> MODER |=  ((2 << 4) | (2 << 6)); //AF mode

	//AFRL: PA2 = bits[11:8], PA3 = bits[15:12]. AF7 = 0x7
	GPIOA -> AFR[0] &= ~((0xF << 8) | (0xF << 12));
	GPIOA -> AFR[0] |=  ((7 << 8) | (7 << 12)); //AF7 for USART2

	//BRR: 16MHz / 9600 = 1667
	USART2 -> BRR = 1667;

	USART2 -> CR1 |= (1 << 5);              //RXNEIE -- enable RXNE interrupt
	USART2 -> CR1 |= (1 << 3);              //TE -- transmitter enable
	USART2 -> CR1 |= (1 << 2);              //RE -- receiver enable
	USART2 -> CR1 |= (1 << 13);             //UE -- USART enable

	NVIC_EnableIRQ(USART2_IRQn);             //Unmask USART2 IRQ in NVIC
}


void TIM1_init(){
	RCC -> APB2ENR |= 1;

	TIM1 -> PSC = 15;           //PSC + 1 = 16. 16MHz / 16 = 1us per tick
	TIM1 -> EGR |= (1 << 0);   //Trigger an event so the prescalar is loaded
	TIM1 -> SR = 0;             //Clear Status register
	TIM1 -> CR1 |= (1 << 3);   //Set one pulse mode so clock disables itself
}

void _delay_us(int n){
	TIM1 -> CNT = 0;            //Clear current count
	TIM1 -> ARR = n - 1;        //Set ARR so event triggers at specified interval
	TIM1 -> SR = 0;             //Clear the status register
	TIM1 -> CR1 |= (1 << 0);   //Enable counter in control register
	while(!(TIM1 -> SR & (1 << 0))){}  //Wait for update flag
}

void SysTick_Init(void){
	SysTick -> LOAD = (SystemCoreClock / 1000) - 1;
	SysTick -> VAL = 0;
	SysTick -> CTRL &= ~(0x07);
	SysTick -> CTRL |= 0x05;
}

void _sysTick_delay_ms(int n){
	int i;
	for(i = 0; i < n; i++){
		while((SysTick -> CTRL & 0x10000) == 0){}
	}
}


void LCD_Pulse_EN(void){
	GPIOB -> BSRR = (1 << 7);           //Set PB7 (EN) HIGH
	_sysTick_delay_ms(10);               //Wait for LCD to read
	GPIOB -> BSRR = (1 << (7 + 16));    //Set PB7 (EN) LOW
	_sysTick_delay_ms(2);                //Wait for command to execute
}

void LCD_Command(unsigned char cmd){
	GPIOB -> BSRR = (1 << (5 + 16));    //RS = 0 for command (PB5 LOW)
	GPIOB -> BSRR = (1 << (6 + 16));    //RW = 0 for write (PB6 LOW)

	//Put 8-bit command onto Port C (PC0-PC7), preserve PC8-PC15
	GPIOC -> ODR = (GPIOC -> ODR & 0xFF00) | cmd;

	LCD_Pulse_EN();                       //Latch the command
}

void LCD_Init_8Bit(void){
	//1. Enable clocks for Port B (control) and Port C (data)
	RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

	//2. Configure PB5, PB6, PB7 as general purpose outputs
	GPIOB -> MODER &= ~((3 << 10) | (3 << 12) | (3 << 14));
	GPIOB -> MODER |= ((1 << 10) | (1 << 12) | (1 << 14));

	//3. Configure PC0 to PC7 as general purpose outputs
	GPIOC -> MODER &= 0xFFFF0000;       //Clear mode bits for PC0-PC7
	GPIOC -> MODER |= 0x00005555;       //Set PC0-PC7 to output mode

	//4. LCD Boot-up Sequence
	_sysTick_delay_ms(20);              //Wait >15ms after power on

	LCD_Command(0x30);                  //Wake up / Function set
	_sysTick_delay_ms(5);               //Wait >4.1ms

	LCD_Command(0x30);                  //Wake up 2
	_delay_us(150);                     //Wait >100us

	LCD_Command(0x30);                  //Wake up 3

	//5. Initial Display Configuration
	LCD_Command(0x38);                  //Function Set: 8-bit, 2 lines, 5x8 font
	LCD_Command(0x0C);                  //Display ON, Cursor OFF, Blink OFF
	LCD_Command(0x01);                  //Clear Display
	_sysTick_delay_ms(2);               //Clear display requires extra time
	LCD_Command(0x06);                  //Entry Mode: auto-increment, no shift
}

void LCD_Data(unsigned char data){
	GPIOB -> BSRR = (1 << 5);           //RS = 1 for data (PB5 HIGH)
	GPIOB -> BSRR = (1 << (6 + 16));    //RW = 0 for write (PB6 LOW)

	//Put 8-bit character data onto Port C (PC0-PC7), preserve PC8-PC15
	GPIOC -> ODR = (GPIOC -> ODR & 0xFF00) | data;

	LCD_Pulse_EN();                       //Latch the data
}

void LCD_String(char* str){
	while(*str){                          //Loop until null terminator '\0'
		LCD_Data(*str);                   //Send the current character
		str++;                            //Move to next character
	}
}
