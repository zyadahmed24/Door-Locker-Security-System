/******************************************************************************
 *
 * Module: DC_Motor
 *
 * File Name: dcmotor.h
 *
 * Description: Source file for the DC_Motor driver
 *
 * Author: Zyad Ahmed
 *
 *******************************************************************************/
#ifndef DCMOTOR_H_
#define DCMOTOR_H_

#include "std_types.h"
#include "gpio.h"

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define CONTROL_PORT PORTB_ID

#define INPUT_PIN_1 PIN1_ID
#define INPUT_PIN_2 PIN2_ID

/*******************************************************************************
 *                                ENUMS                                        *
 *******************************************************************************/

typedef enum{
	CW,A_CW
}DcMotor_State;

/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

void DCMOTOR_init(void);

void DCMOTOR_rotate(DcMotor_State state,uint8 speed);


#endif /* DCMOTOR_H_ */
