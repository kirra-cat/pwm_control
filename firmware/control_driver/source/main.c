/*
 * main.c
 *
 *  Created on: 15 may 2016
 *      Author: Sergey S. Sklyarov (kirra)
 */

#include <stdbool.h>
#include <stddef.h>

#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_usart.h>

#include "Initialization.h"
#include "Interrupts.h"
#include "wh1602.h"
#include "OneWire.h"


void __Delay(uint32_t t)
{
	for (; t > 0; t--);
}

int main (void)
{
	RCC_Initialization();
	GPIO_Initialization();
	LCD44780_Initialization();
	Timers_Initialization();
	USART_Register_Initializiation();
//	USART_Initialization();
	NVIC_Initialization();
//	OneWire_Initialization();
	SysTick_Initialization();

//	__enable_irq();

	LCD44780_SetCursorPosition(0, 0);
	LCD44780_ShowString("T=");
	LCD44780_SetCursorPosition(8, 0);
	LCD44780_ShowString("R=");
	LCD44780_SetCursorPosition(0, 1);
	LCD44780_ShowString("Time:");

	while (true) {	}

	return 0 ;
}

/*
 * end file main.c
 */
