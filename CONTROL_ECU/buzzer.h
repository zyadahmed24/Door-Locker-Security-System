 /******************************************************************************
 *
 * Module: BUZZER
 *
 * File Name: buzzer.h
 *
 * Description: Header file for the Buzzer driver
 *
 * Author: Zyad Ahmed
 *
 *******************************************************************************/
#ifndef BUZZER_H_
#define BUZZER_H_

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define BUZZER_PORT PORTA_ID
#define BUZZER_PIN  PIN3_ID


/*******************************************************************************
 *                              Functions Prototypes                           *
 *******************************************************************************/

void BUZZER_init();
void BUZZER_on();
void BUZZER_off();


#endif /* BUZZER_H_ */
