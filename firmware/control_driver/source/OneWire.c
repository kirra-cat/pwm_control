/*
 * onewire.c
 *
 *  Created on: 13.02.2012
 *      Author: di
 */

#include "OneWire.h"

#include <stm32f0xx.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_dma.h>
#include <stm32f0xx_usart.h>

#ifdef ONE_WIRE_USART1

#undef OW_USART2
#undef OW_USART3
#undef OW_USART4

#define ONE_WIRE_USART	 		USART1
#define ONE_WIRE_DMA_CH_RX 		DMA1_Channel3
#define ONE_WIRE_DMA_CH_TX 		DMA1_Channel2
#define ONE_WIRE_DMA_FLAG		DMA1_FLAG_TC3

#endif


#ifdef ONE_WIRE_USART2

#undef ONE_WIRE_USART1
#undef OW_USART3
#undef OW_USART4

#define ONE_WIRE_USART 		USART2
#define ONE_WIRE_DMA_CH_RX 	DMA1_Channel6
#define ONE_WIRE_DMA_CH_TX 	DMA1_Channel7
#define ONE_WIRE_DMA_FLAG		DMA1_FLAG_TC6

#endif


// Буфер для приема/передачи по 1-wire
uint8_t one_wire_ds18b20_buffer[8];

#define ONE_WIRE_LOW	0x00
#define ONE_WIRE_HIGH	0xff
#define ONE_WIRE_R		0xff

//-----------------------------------------------------------------------------
// функция преобразует один байт в восемь, для передачи через USART
// ow_byte - байт, который надо преобразовать
// ow_bits - ссылка на буфер, размером не менее 8 байт
//-----------------------------------------------------------------------------
void OneWire_ToBits(uint8_t one_wire_byte, uint8_t *one_wire_bits)
{
	uint8_t i;
	for (i = 0; i < 8; i++) {
		if (one_wire_byte & 0x01) {
			*one_wire_bits = ONE_WIRE_HIGH;
		} else {
			*one_wire_bits = ONE_WIRE_LOW;
		}
		one_wire_bits++;
		one_wire_byte = one_wire_byte >> 1;
	}
}

//-----------------------------------------------------------------------------
// обратное преобразование - из того, что получено через USART опять собирается байт
// ow_bits - ссылка на буфер, размером не менее 8 байт
//-----------------------------------------------------------------------------
uint8_t OneWire_ToByte(uint8_t *one_wire_bits)
{
	uint8_t one_wire_byte, i;
	one_wire_byte = 0;
	for (i = 0; i < 8; i++) {
		one_wire_byte = one_wire_byte >> 1;
		if (*one_wire_bits == ONE_WIRE_R) {
			one_wire_byte |= 0x80;
		}
		one_wire_bits++;
	}

	return one_wire_byte;
}

//-----------------------------------------------------------------------------
// инициализирует USART и DMA
//-----------------------------------------------------------------------------
uint8_t OneWire_Initialization()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	if (ONE_WIRE_USART == USART1) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		// USART TX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStructure);

		// USART RX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
		GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

		RCC_USARTCLKConfig(RCC_USART1CLK_SYSCLK);
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	}

	if (ONE_WIRE_USART == USART2) {
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStructure);

		RCC_USARTCLKConfig(RCC_USART2CLK_PCLK);
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	}

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

	USART_Init(ONE_WIRE_USART, &USART_InitStructure);
	USART_Cmd(ONE_WIRE_USART, ENABLE);
	return ONE_WIRE_OK;
}

//-----------------------------------------------------------------------------
// осуществляет сброс и проверку на наличие устройств на шине
//-----------------------------------------------------------------------------
uint8_t OneWire_Reset() {
	uint8_t one_wire_presence;
	USART_InitTypeDef USART_InitStructure;

	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(ONE_WIRE_USART, &USART_InitStructure);

	// отправляем 0xf0 на скорости 9600
	USART_ClearFlag(ONE_WIRE_USART, USART_FLAG_TC);
	USART_SendData(ONE_WIRE_USART, 0xf0);
	while (USART_GetFlagStatus(ONE_WIRE_USART, USART_FLAG_TC) == RESET);

	one_wire_presence = USART_ReceiveData(ONE_WIRE_USART);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(ONE_WIRE_USART, &USART_InitStructure);

	if (one_wire_presence != 0xf0)
	{
		return ONE_WIRE_OK;
	}

	return ONE_WIRE_NO_DEVICE;
}

void OneWire_Out_Set_As_TX_pin(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	if (ONE_WIRE_USART == USART1)
	{
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		// USART TX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	}

	if (ONE_WIRE_USART == USART2)
	{
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStructure);

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	}
}

void OneWire_Out_Set_As_Power_Pin(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	if (ONE_WIRE_USART == USART1)
	{
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		// USART TX
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}

	if (ONE_WIRE_USART == USART2)
	{
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
		GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

		GPIO_Init(GPIOA, &GPIO_InitStructure);
	}
}


//-----------------------------------------------------------------------------
// процедура общения с шиной 1-wire
// sendReset - посылать RESET в начале общения.
// 		OW_SEND_RESET или OW_NO_RESET
// command - массив байт, отсылаемых в шину. Если нужно чтение - отправляем OW_READ_SLOTH
// cLen - длина буфера команд, столько байт отошлется в шину
// data - если требуется чтение, то ссылка на буфер для чтения
// dLen - длина буфера для чтения. Прочитается не более этой длины
// readStart - с какого символа передачи начинать чтение (нумеруются с 0)
//		можно указать OW_NO_READ, тогда можно не задавать data и dLen
//-----------------------------------------------------------------------------
uint8_t OneWire_Send(uint8_t sendReset, uint8_t *command, uint8_t cLen,
		uint8_t *data, uint8_t dLen, uint8_t readStart) {

	// если требуется сброс - сбрасываем и проверяем на наличие устройств
	if (sendReset == ONE_WIRE_SEND_RESET)
	{
		if (OneWire_Reset() == ONE_WIRE_NO_DEVICE)
		{
			return ONE_WIRE_NO_DEVICE;
		}
	}

	while (cLen > 0) {

		OneWire_ToBits(*command, one_wire_ds18b20_buffer);
		command++;
		cLen--;

		DMA_InitTypeDef DMA_InitStructure;

		// DMA на чтение
		DMA_DeInit(ONE_WIRE_DMA_CH_RX);
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USART1->RDR);
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) one_wire_ds18b20_buffer;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
		DMA_InitStructure.DMA_BufferSize = 8;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		DMA_Init(ONE_WIRE_DMA_CH_RX, &DMA_InitStructure);

		// DMA на запись
		DMA_DeInit(ONE_WIRE_DMA_CH_TX);
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USART1->TDR);
		DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t) one_wire_ds18b20_buffer;
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		DMA_InitStructure.DMA_BufferSize = 8;
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
		DMA_Init(ONE_WIRE_DMA_CH_TX, &DMA_InitStructure);

		// старт цикла отправки
		USART_ClearFlag(ONE_WIRE_USART, USART_FLAG_RXNE | USART_FLAG_TC | USART_FLAG_TXE);
		USART_DMACmd(ONE_WIRE_USART, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
		DMA_Cmd(ONE_WIRE_DMA_CH_RX, ENABLE);
		DMA_Cmd(ONE_WIRE_DMA_CH_TX, ENABLE);

		// Ждем, пока не примем 8 байт
		while (DMA_GetFlagStatus(ONE_WIRE_DMA_FLAG) == RESET);

		// отключаем DMA
		DMA_Cmd(ONE_WIRE_DMA_CH_TX, DISABLE);
		DMA_Cmd(ONE_WIRE_DMA_CH_RX, DISABLE);
		USART_DMACmd(ONE_WIRE_USART, USART_DMAReq_Tx | USART_DMAReq_Rx, DISABLE);

		// если прочитанные данные кому-то нужны - выкинем их в буфер
		if (readStart == 0 && dLen > 0)
		{
			*data = OneWire_ToByte(one_wire_ds18b20_buffer);
			data++;
			dLen--;
		}
		else
		{
			if (readStart != ONE_WIRE_NO_READ)
			{
				readStart--;
			}
		}
	}

	return ONE_WIRE_OK;
}
