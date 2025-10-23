/*
 * uart.h
 *
 *  Created on: Oct 25, 2022
 *      Author: hp
 */

#ifndef UART_H_
#define UART_H_

#include "std_types.h"

/*******************************************************************************
 *                                  Definitions                                *
 *******************************************************************************/

#define FREQUENCY 8000000UL
#define STRING_END '#'


/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/

typedef enum{
	Bit_5,
	Bit_6,
	Bit_7,
	Bit_8,
	Bit_9=7
}UART_BitData;

typedef enum{
	Disabled,
	Reserved,
	EvanParity,
	OddParity
}UART_Parity;

typedef enum{
	OneStopBit,
	TwoStopBits
}UART_StopBit;

typedef uint32 UART_BaudRate;

typedef struct{
	UART_BitData bit_data;
	UART_Parity parity;
	UART_StopBit stop_bit;
	UART_BaudRate baud_rate;
}UART_ConfigType;

/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

void UART_init(UART_ConfigType* configs);
void UART_sendByte(uint8 data);
uint8 UART_receiveByte();
void UART_sendString(uint8 str[]);
void UART_receiveString(uint8 restr[]);


#endif /* UART_H_ */
