 /******************************************************************************
 *
 * Module: TIMER1
 *
 * File Name: timer1_driver.c
 *
 * Description: Source file for the AVR Tmer1 driver
 *
 * Author: Zyad Ahmed
 *
 *******************************************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer1_driver.h"
#include "std_types.h"
#include "common_macros.h"

/*******************************************************************************
 *                                 Globals                                     *
 *******************************************************************************/

void (*gptr)();

/*******************************************************************************
 *                                    ISRs                                     *
 *******************************************************************************/

ISR(TIMER1_COMPA_vect)
{
	(*gptr)();
	// Operate the upper layer function.
}

ISR(TIMER1_OVF_vect)
{
	(*gptr)();
	// Operate the upper layer function.
}


/*******************************************************************************
 *                              Functions Definition                           *
 *******************************************************************************/

void TIMER1_init(Timer1_ConfigType* configs)
{
	SET_BIT(TCCR1A,FOC1A); // Set this bit as long as no PWM.
	SET_BIT(TCCR1A,FOC1B); // Set this bit as long as no PWM.

	TCCR1B = (TCCR1B & 0xF7) | (configs->mode << 3);
	// The WGM12 is the only difference between Overflow mode and Compare mode.

	TCCR1B = (TCCR1B & 0xF8) | (configs->prescaler);
	// Put up the prescalar on CS10,CS11,CS12 bits.

	TCNT1 = configs->initial_vlaue; // Setup the initial value.
	OCR1A = configs->compare_value; // Setup the compare value.

	if(configs->mode == NORMAL)
	{
		SET_BIT(TIMSK,TOIE1);
		// if the normal mode is selected, Set the interrupt enable of the overflow mode.
	}
	else
	{
		SET_BIT(TIMSK,OCIE1A);
		// if the compare mode is selected, Set the interrupt enable of the compare mode.
	}
}
void TIMER1_deinit()
{
	TCCR1A=0;
	TCCR1B=0;

	TCNT1=0;
	OCR1A=0;

	CLEAR_BIT(TIMSK,TOIE1);
	//Regardless the mode, Clear the OVERFLOW interrupt enable.
	CLEAR_BIT(TIMSK,OCIE1A);
	//Regardless the mode, Clear the COMPARE interrupt enable.
}
void TIMER1_setCallBack(void(*ptr)())
{
	gptr = ptr;
	// Make the global pointer points to the upper layer function.
}
