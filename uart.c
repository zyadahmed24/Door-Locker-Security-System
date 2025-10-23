/*
 * uart.c
 *
 *  Created on: Oct 25, 2022
 *      Author: hp
 */

#include <avr/io.h>
#include "uart.h"
#include "gpio.h"
#include "common_macros.h"

void UART_init(UART_ConfigType* configs)
{
	SET_BIT(UCSRA,U2X); // Double transmission speed activated.
	CLEAR_BIT(UCSRA,MPCM); // Multi-Processor communication mode activated.

	SET_BIT(UCSRB,RXEN); // Enable the receiving.
	SET_BIT(UCSRB,TXEN); // Enable the transmitting.
	UCSRB = (UCSRB & 0xFB) | ((configs->bit_data & (1<<2))<<2);

	UCSRC = (UCSRC & 0xFC) | ((configs->bit_data & 0x03)<<1);
	CLEAR_BIT(UCSRC,URSEL);
	CLEAR_BIT(UCSRC,UMSEL);
	UCSRC = (UCSRC & 0xCF) | ((configs->parity & 0x03)<<4);
	UCSRC = (UCSRC & 0xF7) | ((configs->stop_bit)<<3);
	CLEAR_BIT(UCSRC,UCPOL);

	UBRRL = (uint8)(((uint32)FREQUENCY/(8*configs->baud_rate))-1);

	GPIO_setupPinDirection(PORTD_ID, PIN1_ID, PIN_OUTPUT);
	GPIO_setupPinDirection(PORTD_ID, PIN0_ID, PIN_INPUT);
}

void UART_sendByte(uint8 data)
{
	while(BIT_IS_CLEAR(UCSRA,UDRE));
	UDR=data;
}

uint8 UART_receiveByte()
{
	while(BIT_IS_CLEAR(UCSRA,RXC));
	return UDR;
}

void UART_sendString(uint8 str[])
{
	uint8 i=0;
	while(str[i] != '\0')
	{
		UART_sendByte(*(str+i));
		i++;
	}
}

void UART_receiveString(uint8 restr[])
{
	uint8 i=0;
	restr[i]=UART_receiveByte();

	while(restr[i] != STRING_END)
	{
		i++;
		restr[i]=UART_receiveByte();
	}
	restr[i]='\0';

}
