/******************************************************************************
 *
 * Module: DC_Motor
 *
 * File Name: dcmotor.c
 *
 * Description: Source file for the DC_Motor driver
 *
 * Author: Zyad Ahmed
 *
 *******************************************************************************/

/*******************************************************************************
 *                                INCLUDES                                     *
 *******************************************************************************/

#include "dcmotor.h"
#include <avr/io.h>

/*******************************************************************************
 *                                FUNCTIONS                                    *
 *******************************************************************************/

static void PWM_Timer0_Start(uint8 duty_cycle)
{
	TCCR0 |= (1<<WGM00) | (1<<WGM01) ;
	TCCR0 |= (1<<COM01) ;
	TCCR0 |= (1<<CS01) | (1<<CS00);

	TCNT0 = 0;
	OCR0 = (duty_cycle*255)/100;

	GPIO_setupPinDirection(PORTB_ID, PIN3_ID, PIN_OUTPUT);
}

/*------------------------------------------------------------------------------
 * [Function Name]: DCMOTOR_init
 * [Description]: Set the pin direction of the motor pins
 * [Args]: Void
 * [in]: Void
 * [out]: Void
 * [Returns]: Void
 -----------------------------------------------------------------------------*/
void DCMOTOR_init(void)
{
	GPIO_setupPinDirection(CONTROL_PORT, INPUT_PIN_1, PIN_OUTPUT);
	GPIO_setupPinDirection(CONTROL_PORT, INPUT_PIN_2, PIN_OUTPUT);

	GPIO_writePin(CONTROL_PORT, INPUT_PIN_1, LOGIC_LOW);
	GPIO_writePin(CONTROL_PORT, INPUT_PIN_2, LOGIC_LOW);
}

/*------------------------------------------------------------------------------
 * [Function Name]: DCMOTOR_rotate
 * [Description]: Out the pins to control the direction of the motion
 * [Args]: state and speed
 * [in]: state->{clock wise,counter clock wise}
 * 		 speed-> duty cycle of the PWM
 * [out]: Void
 * [Returns]: Void
 -----------------------------------------------------------------------------*/
void DCMOTOR_rotate(DcMotor_State state,uint8 speed)
{
	if(state == CW)
	{
		GPIO_writePin(CONTROL_PORT, INPUT_PIN_1, LOGIC_LOW);
		GPIO_writePin(CONTROL_PORT, INPUT_PIN_2, LOGIC_HIGH);
	}
	else if(state == A_CW)
	{
		GPIO_writePin(CONTROL_PORT, INPUT_PIN_1, LOGIC_HIGH);
		GPIO_writePin(CONTROL_PORT, INPUT_PIN_2, LOGIC_LOW);
	}

	PWM_Timer0_Start(speed);
}
