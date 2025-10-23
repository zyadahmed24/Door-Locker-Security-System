 /******************************************************************************
 *
 * Module: TIMER1
 *
 * File Name: timer1_driver.h
 *
 * Description: Header file for the AVR Tmer1 driver
 *
 * Author: Zyad Ahmed
 *
 *******************************************************************************/
#ifndef TIMER1_DRIVER_H_
#define TIMER1_DRIVER_H_

#include "std_types.h"

/*******************************************************************************
 *                               Types Declaration                             *
 *******************************************************************************/

typedef enum{
	NO_CLOCK,
	F_CPU_1,
	F_CPU_8,
	F_CPU_64,
	F_CPU_256,
	F_CPU_1024
}Timer1_Prescaler;

typedef enum{
	NORMAL,
	COMPARE
}Timer1_Mode;

typedef struct{
	uint16 initial_vlaue;
	uint16 compare_value; // it will be used in compare mode.
	Timer1_Prescaler prescaler;
	Timer1_Mode mode;
}Timer1_ConfigType;



/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/
/*
 * Description:
 * Select the needed mode for timer1.
 * Setup the initial and compare value.
 * Setup the prescaler of timer1.
 *
 * Note:
 * The compare mode deals with OCR1A.
 */
void TIMER1_init(Timer1_ConfigType* configs);

/*
 * Description:
 * Clears the overflow and compare interrupt enable bits.
 */
void TIMER1_deinit();

/*
 * Description:
 * Takes the upper layers functions.
 */
void TIMER1_setCallBack(void(*ptr)());

#endif /* TIMER1_DRIVER_H_ */
