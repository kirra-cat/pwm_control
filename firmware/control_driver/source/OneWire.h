/*
 * onewire.h
 *
 *  Version 1.0.1
 */

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

// для разных процессоров потребуется проверить функцию OW_Init
// на предмет расположения ножек USART
#include "stm32f0xx.h"

// выбираем, на каком USART находится 1-wire
#define ONE_WIRE_USART1
/** #define OW_USART2 */
//#define OW_USART3
//#define OW_USART4


// если нужно отдавать тики FreeRTOS, то раскомментировать
//#define OW_GIVE_TICK_RTOS

// первый параметр функции OW_Send
#define ONE_WIRE_SEND_RESET		1
#define ONE_WIRE_NO_RESET		2

// статус возврата функций
#define ONE_WIRE_OK			0
#define ONE_WIRE_ERROR		2
#define ONE_WIRE_NO_DEVICE	3

#define ONE_WIRE_NO_READ	0xff

#define ONE_WIRE_READ_SLOT	0xff

uint8_t OneWire_Initialization();
uint8_t OneWire_Send(uint8_t sendReset, uint8_t *command, uint8_t cLen, uint8_t *data, uint8_t dLen, uint8_t readStart);

#endif /* ONEWIRE_H_ */
