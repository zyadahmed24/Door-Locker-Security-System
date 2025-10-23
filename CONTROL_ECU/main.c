 /******************************************************************************
 *
 * File Name: main.c
 *
 * Description: CONTROL_ECU application file
 *
 * Author: Zyad Ahmed
 *
 *******************************************************************************/

#include "dcmotor.h"
#include "timer1_driver.h"
#include "buzzer.h"
#include "uart.h"
#include "std_types.h"
#include "twi.h"
#include "external_eeprom.h"
#include "gpio.h"
#include "common_macros.h"
#include <util/delay.h>
#include <avr/io.h>

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define F_CPU 8000000UL
#define READY_TO_RECEIVE 0x02
#define MATCHED   0x09
#define UNMATCHED 0x07
#define FULL_SPEED 100
#define OPEN_DOOR 0x10
#define CHANGE_PASSWORD 0x20
#define EEPROM_ADDRESS 0x0100

/*******************************************************************************
 *                                   Globals                                   *
 *******************************************************************************/

uint8 First_Password[7];
uint8 Second_Password[7];
uint8 Operating_Password[7];
uint8 Error_flag=0;
uint8 Error_flag_after=0;
uint8 Error_flag_ChangePass=0;
uint8 operation=0;
uint8 timer_tickes=0;
uint8 timer_flag=0;
uint8 wrong_counter=0;
uint8 int_3errors_ISR_flag=0;
uint8 ext_3errors_ISR_flag=1;

/*******************************************************************************
 *                         Timer CallBack Function                             *
 *******************************************************************************/

uint8 tick_door=0;
void FunToTimer_Door()
{
	tick_door++;
	if(tick_door < 15)
	{
		DCMOTOR_rotate(CW, 100);
	}
	else if(15<=tick_door && tick_door<18)
	{
		DCMOTOR_rotate(CW, 0);
	}
	else if(18<=tick_door && tick_door<33)
	{
		DCMOTOR_rotate(A_CW, 100);
	}
	else
	{
		tick_door=0;
		DCMOTOR_rotate(CW, 0);
		TIMER1_deinit();
	}
}

uint8 tick_3errors=0;
void FunToTimer_3Errors()
{
	tick_3errors++;
	if(!int_3errors_ISR_flag)
	{
		BUZZER_on();
		int_3errors_ISR_flag=1;
		ext_3errors_ISR_flag+=2;
	}
	if(tick_3errors==60)
	{
		BUZZER_off();
		int_3errors_ISR_flag=0;
		ext_3errors_ISR_flag++;

		TIMER1_deinit();
	}
}

/*******************************************************************************
 *                         Functions Used In Main                              *
 *******************************************************************************/

void StoreThePasswordInEEPROM(uint8 *pw)
{
	uint8 i=0;
	for(i=0;i<=4;i++)
	{
		EEPROM_writeByte(EEPROM_ADDRESS+i, pw[i]); /* Write 0x0F in the external EEPROM */
		_delay_ms(10);
	}
}

uint8 ComparePasswordAndEEPROM(uint8 *pw)
{
	uint8 i=0;
	uint8 EEval=0;
	for(i=0;i<=4;i++)
	{
		EEPROM_readByte(EEPROM_ADDRESS+i, &EEval);
		_delay_ms(10);
		if(pw[i] != EEval)
		{
			return 0;
		}
	}
	return 1;
}

void CheckTwoPassWords(uint8 pw1[],uint8 pw2[])
{
	uint8 i=0;
	for(i=0;i<=4;i++)
	{
		if(pw1[i]!=pw2[i])
		{
			Error_flag=1;
		}
	}
}

void OpenDoor()
{
	SET_BIT(SREG,7);

	Timer1_ConfigType configs={0,31250,F_CPU_256,COMPARE};
	TIMER1_init(&configs);

	TIMER1_setCallBack(FunToTimer_Door);
}

void ChengePassWord()
{
	while(1)
	{
		UART_sendByte(READY_TO_RECEIVE);
		UART_receiveString(First_Password);

		UART_sendByte(READY_TO_RECEIVE);
		UART_receiveString(Second_Password);

		uint8 i=0;
		for(i=0;i<=4;i++)
		{
			if(First_Password[i]!=Second_Password[i])
			{
				Error_flag_ChangePass=1;
			}
		}


		if(!Error_flag_ChangePass)
		{
			while(UART_receiveByte() != READY_TO_RECEIVE);
			UART_sendByte(MATCHED);

			StoreThePasswordInEEPROM(First_Password);
			break;
		}
		else
		{
			while(UART_receiveByte() != READY_TO_RECEIVE);
			UART_sendByte(UNMATCHED);
			Error_flag_ChangePass=0;
		}
	}
}

void ThreeTimesError()
{
	while(UART_receiveByte() != READY_TO_RECEIVE);
	UART_sendByte(UNMATCHED);
	wrong_counter++;
	if(wrong_counter>2)
	{
		SET_BIT(SREG,7);

		Timer1_ConfigType configs={0,31250,F_CPU_256,COMPARE};
		TIMER1_init(&configs);

		TIMER1_setCallBack(FunToTimer_3Errors);
		while( (ext_3errors_ISR_flag%2) != 0);
		wrong_counter=0;
	}
}


/*******************************************************************************
 *                                Main Functions                               *
 *******************************************************************************/

int main(void)
{
	UART_ConfigType configs={Bit_8,Disabled,OneStopBit,9600};	/* The configurations struct of the UART */
	UART_init(&configs);										/* Initialize the UART */

	TWI_ConfigType configrations={0b00000010,Prescaler_1};		/* The configurations struct of the TWI */
	TWI_init(&configrations);									/* Initialize the TWI */

	DCMOTOR_init();
	BUZZER_init();

	while(1)
	{
		UART_sendByte(READY_TO_RECEIVE);		/* Send to the HMI_ECU that you are ready to receive */
		UART_receiveString(First_Password);		/* Receive the first password */

		UART_sendByte(READY_TO_RECEIVE);		/* Send to the HMI_ECU that you are ready to receive */
		UART_receiveString(Second_Password);	/* Receive the first password */

		CheckTwoPassWords(First_Password, Second_Password);		/* Check the two passwords */


		if(!Error_flag)										/* Based on the Checking function the Error_flag is changed */
		{													/* if the two received passwords are matched Go in */
			while(UART_receiveByte() != READY_TO_RECEIVE);  /* Wait till the C_ECU send that he is ready */
			UART_sendByte(MATCHED);							/* Send the feedback to HMI_ECU */

			StoreThePasswordInEEPROM(First_Password);		/* Store one of the Passwords in the EEPROM */

			while(1)
			{
				UART_sendByte(READY_TO_RECEIVE);			/* Send to the HMI_ECU that you are ready to receive */
				UART_receiveString(Operating_Password);		/* Receive the entered password to HMI_ECU */

				if(ComparePasswordAndEEPROM(Operating_Password) == 1)  /* Compare the password to the password stored in EEPROM */
				{
					while(UART_receiveByte() != READY_TO_RECEIVE);		/* Wait till the HMI_ECU send that he is ready */
					UART_sendByte(MATCHED);								/* Send the feedback */

					UART_sendByte(READY_TO_RECEIVE);					/* Send that your are ready to receive the operation */
					operation=UART_receiveByte();						/* Receive the operation */

					if(operation==OPEN_DOOR)			/* If the received is Open Door */
					{
						OpenDoor();						/* Perform the OpenDoor routine */
					}
					else if(operation==CHANGE_PASSWORD) /* If the received is Open Door */
					{
						ChengePassWord();				/* Perform the ChangePassWord routine */
					}

				}
				else					/* If the entered password does not match the EEPROM */
				{
					ThreeTimesError();	/* Perform the ThreeTimeError routine */
				}
			}
		}
		else
		{
			while(UART_receiveByte() != READY_TO_RECEIVE);	/* Wait till the HMI_ECU send that he is ready */
			UART_sendByte(UNMATCHED);						/* Send that the initially two passwords are not matched */

			Error_flag=0;			/* Clear the Error_flag to use it in the check process in the next times */

			for(uint8 i=0;i<=4;i++)
			{
				First_Password[i]=0; /* Clear the two passwords */
				Second_Password[i]=0;
			}
		}

	}
}


