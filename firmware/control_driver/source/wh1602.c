/*
 * wh1602.c
 *
 *  Created on: 24 мая 2016 г.
 *      Author: Sergey S.Sklayrov <kirra>
 */

#include <stdint.h>

#include <stm32f0xx.h>
#include <stm32f0xx_gpio.h>

#include "wh1602.h"

const uint8_t  lcd44780_addLUT[4] = {0x80, 0xC0, 0x94, 0xD4};
volatile uint8_t lcd44780_Address, lcd44780_Line;

void LCD44780_Delay(uint32_t t)
{
	for (; t > 0; t--);
}

void LCD44780_WriteNibble(uint8_t data)
{
	GPIO_SetBits(GPIOB,((data & 0x0F)) << LCD44780_DATA_OFFSET);
	LCD44780_Delay(2000);
	GPIO_SetBits(GPIOB, LCD44780_PIN_E);
	LCD44780_Delay(1000);
	GPIO_ResetBits(GPIOB, LCD44780_PIN_E);
	GPIO_ResetBits(GPIOB, ((data & 0x0F)) << LCD44780_DATA_OFFSET);
}

void LCD44780_WriteByte(uint8_t data)
{
	LCD44780_WriteNibble(data >> 4);
	LCD44780_WriteNibble(data & 0x0F);
}

void LCD44780_GoToLine(uint8_t LineNum)
{
	GPIO_ResetBits(GPIOB, LCD44780_PIN_RS);
	lcd44780_Address = lcd44780_addLUT[LineNum-1];
	LCD44780_WriteByte(lcd44780_Address);
	GPIO_SetBits(GPIOB, LCD44780_PIN_RS);
	lcd44780_Address = 0;
	lcd44780_Line = LineNum;
}

void LCD44780_ClearDisplay(void)
{
	GPIO_ResetBits(GPIOB, LCD44780_PIN_RS);
	LCD44780_WriteByte(0x01);
	LCD44780_Delay(10000);
	GPIO_SetBits(GPIOB, LCD44780_PIN_RS);
	LCD44780_GoToLine(1);
}

void LCD44780_SetCursorPosition(uint8_t x, uint8_t y)
{
	GPIO_ResetBits(GPIOB, LCD44780_PIN_RS);
	lcd44780_Address = lcd44780_addLUT[y] + x;
	LCD44780_WriteByte(lcd44780_Address);
	GPIO_SetBits(GPIOB, LCD44780_PIN_RS);
	lcd44780_Line = y + 1;
}

void LCD44780_ShowChar(uint8_t c)
{
	GPIO_SetBits(GPIOB, LCD44780_PIN_RS);
	LCD44780_WriteByte(c);
	lcd44780_Address++;
	switch (lcd44780_Address)
	{
		case 20: LCD44780_GoToLine(2); break;
		case 40: LCD44780_GoToLine(3); break;
		case 60: LCD44780_GoToLine(4); break;
		case 80: LCD44780_GoToLine(1); break;
	}
}

void LCD44780_ShowString(uint8_t *s)
{
	while (*s != 0) LCD44780_ShowChar(*s++);
}

void LCD44780_Initialization(void)
{
	uint8_t i;
	GPIO_ResetBits(GPIOB, LCD44780_PIN_E);
	GPIO_ResetBits(GPIOB, LCD44780_PIN_RS);
	LCD44780_Delay(50000);
	LCD44780_WriteNibble(0x33);
	LCD44780_WriteNibble(0x33);
	LCD44780_WriteNibble(0x33);
	LCD44780_WriteNibble(0x22);
	LCD44780_WriteByte(0x28);
	LCD44780_WriteByte(0x01);
	LCD44780_WriteByte(0x10);
	LCD44780_WriteByte(0x06);
	LCD44780_WriteByte(0x0C);
	for(i=0x40; i<0x5F; i++)
	{
		LCD44780_Delay(10000);
		GPIO_ResetBits(GPIOB, LCD44780_PIN_RS);
		LCD44780_WriteByte(i);
		LCD44780_Delay(10000);
		LCD44780_ShowChar(0);
	}
	GPIO_SetBits(GPIOB, LCD44780_PIN_RS);
	LCD44780_ClearDisplay();
}
