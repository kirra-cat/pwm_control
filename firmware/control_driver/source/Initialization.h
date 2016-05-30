/*
 * Initialization.h
 *
 *  Created on: 15 мая 2016 г.
 *      Author: kirra
 */

#ifndef INITIALIZATION_H_
#define INITIALIZATION_H_

void RCC_Initialization(void);

void GPIO_Initialization (void);

void Timers_Initialization (void);

void USART_Initialization(void);

void USART_Register_Initializiation(void);

void NVIC_Initialization(void);

void DMA_Initialization(void);

#endif
