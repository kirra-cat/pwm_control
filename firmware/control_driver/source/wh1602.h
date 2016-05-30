/*
 * wh1602.h
 *
 *  Created on: 24 мая 2016 г.
 *      Author: kirra
 */

#ifndef WH1602_H_
#define WH1602_H_

#define LCD44780_PIN_RS       GPIO_Pin_0 /** PB0 */
#define LCD44780_PIN_RW       GPIO_Pin_0 /** PA0 */
#define LCD44780_PIN_E        GPIO_Pin_1 /** PB1 */

#define LCD44780_DATA         GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 /** PB2-PB5 */
#define LCD44780_DATA_OFFSET  2

void LCD44780_Delay(uint32_t t);

void LCD44780_WriteNibble(uint8_t data);

void LCD44780_WriteByte(uint8_t data);

void LCD44780_GoToLine(uint8_t LineNum);

void LCD44780_ClearDisplay(void);

void LCD44780_SetCursorPosition(uint8_t x, uint8_t y);

void LCD44780_ShowChar(uint8_t c);

void LCD44780_ShowString(uint8_t *s);

void LCD44780_Initialization(void);

#endif /* WH1602_H_ */
