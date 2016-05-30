/*
 * Interrupts.c
 *
 *  Created on: 25 мая 2016 г.
 *      Author: kirra
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <stm32f0xx.h>
#include <stm32f0xx_conf.h>
#include <stm32f0xx_tim.h>
#include <stm32f0xx_gpio.h>

#include "Interrupts.h"
#include "OneWire.h"
#include "wh1602.h"

volatile uint8_t temperature[8];

volatile uint8_t counter_sec_h = 0,
		          counter_sec_l = 0,
		          counter_min_h = 0,
		          counter_min_l = 0,
		          counter_hour_h = 0,
	          	  counter_hour_l = 0;

/** по TIM15 измеряем температуру */
void TIM15_IRQHandler(void)
{
	if (TIM_GetFlagStatus(TIM15, TIM_IT_Update) != RESET)
	{
		//
		//		LCD44780_SetCursorPosition(9, 0);
		//		LCD44780_ShowChar(counter0 + '0');
		//
		//		LCD44780_SetCursorPosition(9, 1);
		//		LCD44780_ShowChar(counter1 + '0');

		//		OneWire_Send(OW_SEND_RESET, "\xcc\x44", 2, NULL, NULL, OW_NO_READ);
		//
		//		uint32_t i = 0;
		//		for (i = 0; i < 1000000; i++);
		//
		//		OneWire_Send(OW_SEND_RESET, "\xcc\xbe\xff\xff", 4, temperature,2, 2);

		//		OneWire_Send(OW_SEND_RESET, "\xcc\x44", 2, NULL, NULL, 0);
		//		// выдерживаем время измерения (например 750 мс для 12-битного измерения)
		//		OneWire_Out_Set_As_Power_Pin();
		//		for (i=0; i<1000000; i++);
		//		OneWire_Out_Set_As_TX_pin();
		//
		//		uint8_t buf[2];
		//		 OneWire_Send(OW_SEND_RESET, "\xcc\xbe\xff\xff", 4, buf,2, 2);
		//
		//		for (i = 0; i < 2; i++)
		//		{
		//			LCD44780_SetCursorPosition(i + 3, 0);
		//			LCD44780_ShowChar(buf[i] + '0');
		//			LCD44780_SetCursorPosition(i + 4, 0);
		//			LCD44780_ShowChar(buf[i] + '0');
		//		}
		//
		//		if (pos_x == 16)
		//		{
		//			pos_x = 0;
		//			pos_y ^= 1;
		//		}
		//		if (counter0 == 9) counter0 = 0;
		//		if (counter1 == 0) counter1 = 9;
		TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
	}
}

/** по TIM14 обновляем значение прошедшего времени на экране */
void TIM14_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM14, TIM_IT_Update) != RESET)
	{
		if (counter_sec_l == 10)
		{
			counter_sec_h++;
			counter_sec_l = 0;
		}

		if (counter_sec_h == 6)
		{
			counter_min_l++;
			counter_sec_h = 0;
		}

		if (counter_min_l == 10)
		{
			counter_min_h++;
			counter_min_l = 0;
		}

		if (counter_min_h == 6)
		{
			counter_hour_l++;
			counter_min_h = 0;
		}

		if (counter_hour_l == 10)
		{
			counter_hour_h++;
			counter_hour_l = 0;
		}

		if (counter_hour_h == 6)
		{
			counter_sec_h = 0;
			counter_sec_l = 0;

			counter_min_h = 0;
			counter_min_l = 0;

			counter_hour_h = 0;
			counter_hour_l = 0;
		}

		LCD44780_SetCursorPosition(6, 1);
		LCD44780_ShowChar(counter_hour_h + '0');
		LCD44780_SetCursorPosition(7, 1);
		LCD44780_ShowChar(counter_hour_l + '0');
		LCD44780_SetCursorPosition(8, 1);
		LCD44780_ShowChar(':');

		LCD44780_SetCursorPosition(9, 1);
		LCD44780_ShowChar(counter_min_h + '0');
		LCD44780_SetCursorPosition(10, 1);
		LCD44780_ShowChar(counter_min_l + '0');
		LCD44780_SetCursorPosition(11, 1);
		LCD44780_ShowChar(':');

		LCD44780_SetCursorPosition(12, 1);
		LCD44780_ShowChar(counter_sec_h+ '0');
		LCD44780_SetCursorPosition(13, 1);
		LCD44780_ShowChar(counter_sec_l + '0');

		counter_sec_l++;

		TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
	}
}

void USART2_IRQHandler(void)
{
	uint8_t data;

	if (USART_GetITStatus(USART2, USART_FLAG_RXNE) == SET)
	{
		data = USART_ReceiveData(USART2);

		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		USART_ClearFlag(USART2, USART_FLAG_RXNE);

		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);

		USART_SendData(USART2, data);

		USART_ClearITPendingBit(USART2, USART_IT_TXE);
		USART_ClearFlag(USART2, USART_FLAG_TXE);
	}
}
