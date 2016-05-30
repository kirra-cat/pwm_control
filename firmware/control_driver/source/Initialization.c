/*
 * Initialization.c
 *
 *  Created on: 15 мая 2016 г.
 *      Author: kirra
 */

#include <stm32f0xx.h>
#include <stm32f0xx_rcc.h>
#include <stm32f0xx_gpio.h>
#include <stm32f0xx_exti.h>
#include <stm32f0xx_usart.h>
#include <stm32f0xx_dma.h>

#include <stm32f0xx_misc.h>

#include "Initialization.h"
#include "wh1602.h"

volatile uint8_t one_wire_ds18b20_buffer[8], usart_cp2102_buffer[8];
static __IO uint32_t TimingDelay;

void RCC_Initialization(void)
{
	ErrorStatus HSEStartUpStatus;

	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE */
	RCC_HSEConfig(RCC_HSE_ON);

	/* Wait till HSE is ready */
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if (HSEStartUpStatus == SUCCESS)
	{
		/* HCLK = SYSCLK */
		RCC_HCLKConfig(RCC_SYSCLK_Div1);

		/* PCLK = HCLK */
		RCC_PCLKConfig(RCC_HCLK_Div1);

		/* PLLCLK = 8MHz * 6 = 48 MHz */
		RCC_PLLConfig(RCC_PLLSource_HSE, RCC_PLLMul_6);

		/* Enable PLL */
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready */
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while (RCC_GetSYSCLKSource() != 0x08);
	}
}

void SysTick_Initialization(void)
{
	SysTick_Config(48000);
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); /** 48/8 -> 6MHz */
}

void SysTick_Handler(void)
{
	if (TimingDelay != 0) TimingDelay--;
}

void SysTick_Delay_ms(__IO uint32_t Timing)
{
	TimingDelay = Timing;
	while(TimingDelay != 0);
}

void GPIO_Initialization (void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Pin = LCD44780_DATA | LCD44780_PIN_E | LCD44780_PIN_RS | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_1);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_1);

	/** PA2, PA9 -> TX UART */
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/** PA3, PA10  -> RX UART. */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_10;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
}

void Delay_ms(volatile __IO uint32_t nTime)
{
	static __IO uint32_t TimingDelay;
	TimingDelay = nTime*568;//1000
	while(TimingDelay != 0) TimingDelay--;
}

void Timers_Initialization (void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM15, ENABLE);

	TIM_TimeBaseInitTypeDef Timer_InitStructure;
	TIM_TimeBaseStructInit(&Timer_InitStructure);

	/* для TIM14 делитель 48000, период 1000 мс.  */
	Timer_InitStructure.TIM_Prescaler = 47999;
	Timer_InitStructure.TIM_Period = 1000;

	TIM_TimeBaseInit(TIM14, &Timer_InitStructure);

	/* для TIM15 делитель 48000, период 500 мс.  */
	Timer_InitStructure.TIM_Period = 500;
	TIM_TimeBaseInit(TIM15, &Timer_InitStructure);

	TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);
	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);

	TIM_Cmd(TIM14, ENABLE);
	TIM_Cmd(TIM15, ENABLE);
}

void USART_Initialization(void)
{
	USART_InitTypeDef USART_InitStructure;
	USART_ClockInitTypeDef USART_ClockInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	USART_Init(USART2, &USART_InitStructure);

	USART_ClockInitStructure.USART_Clock = USART_Clock_Enable;
	USART_ClockInitStructure.USART_CPHA = USART_CPHA_1Edge;
	USART_ClockInitStructure.USART_CPOL = USART_CPOL_High;
	USART_ClockInitStructure.USART_LastBit = USART_LastBit_Enable;
	USART_ClockInit(USART1, &USART_ClockInitStructure);
	USART_ClockInit(USART2, &USART_ClockInitStructure);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	USART_Cmd(USART1, ENABLE);
	USART_Cmd(USART2, ENABLE);
}

void USART_Register_Initializiation(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	USART1->BRR=0x1A0; //BaudRate 115200
	USART1->CR1 |= USART_CR1_UE; //Разрешаем работу USART1
	USART1->CR1 |= USART_CR1_TE; //Включаем передатчик

	USART2->BRR=0x1A0; //BaudRate 115200
	USART2->CR1 |= USART_CR1_UE; //Разрешаем работу USART2
	USART2->CR1 |= USART_CR1_TE; //Включаем передатчик
}

void send_to_uart(uint8_t data)
{
	while(!(USART2->ISR & USART_ISR_TC)); //Ждем пока бит TC в регистре SR станет 1
	USART2->TDR = data; //Отсылаем байт через UART
}

void DMA_Initialization(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_InitTypeDef DMA_InitStructure;
	/** DMA Receive USART1 */
	DMA_DeInit(DMA1_Channel3);
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
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);

	/** DMA Transmit USART1 */
	DMA_DeInit(DMA1_Channel2);
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
	DMA_Init(DMA1_Channel3, &DMA_InitStructure);

	/** DMA Receive USART2 */
	DMA_DeInit(DMA1_Channel6);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USART2->RDR);
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
	DMA_Init(DMA1_Channel6, &DMA_InitStructure);

	/** DMA Transmit USART2 */
	DMA_DeInit(DMA1_Channel7);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) &(USART2->TDR);
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
	DMA_Init(DMA1_Channel7, &DMA_InitStructure);
}

void NVIC_Initialization(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	/* Enable the USARTx Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM15_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 4;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
