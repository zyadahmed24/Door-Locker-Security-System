/******************************************************************************
 *
 * File Name: main.c
 *
 * Description: HMI_ECU application file
 *
 * Author: Zyad Ahmed
 *
 *******************************************************************************/

#include "lcd.h"
#include "keypad.h"
#include "gpio.h"
#include "uart.h"
#include "timer1_driver.h"
#include <util/delay.h>
#include <avr/io.h>

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/

#define ENTER_KEY '='
#define READY_TO_RECEIVE 0x02
#define MATCHED   0x09
#define UNMATCHED 0x07
#define OPEN_DOOR 0x10
#define CHANGE_PASSWORD 0x20
#define F_CPU 8000000UL

/*******************************************************************************
 *                                   Globals                                   *
 *******************************************************************************/

uint8 First_Password[7];
uint8 Second_Password[7];
uint8 Operating_Password[7];

uint8 feedback=0;
uint8 feedback_mainOptions=0;
uint8 feedback_ChangePass=0;
uint8 flag=1;
uint8 wrong_counter=0;
uint8 operation=0;

uint8 internal_ISR_flag=0;
uint8 external_ISR_flag=1;

uint8 internal_ISR_flag_Door1=0;
uint8 internal_ISR_flag_Door2=0;
uint8 internal_ISR_flag_Door3=0;
uint8 external_ISR_flag_Door=1;

/*
 * The idea of the external flags is that
 * when I need the program to reject any input by while loop
 * I can not do it by binary flag (0,1) as it will work one time
 * and will not work for the second time
 * I did it by (%2) to do it in all times.
 */

/*******************************************************************************
 *                         Timer CallBack Function                             *
 *******************************************************************************/

uint8 tick_door=0;
void FunToTimer_Door()
{
	tick_door++;
	if(tick_door < 15)
	{
		if(!internal_ISR_flag_Door1)
		{
			LCD_sendCommand(LCD_CLEAR_COMMAND);
			LCD_moveCursor(0, 0);
			LCD_displayStrig("Door is Unlocking");
			internal_ISR_flag_Door1=1;
			external_ISR_flag_Door+=2;
		}
	}
	else if(15<=tick_door && tick_door<18)
	{
		if(!internal_ISR_flag_Door2)
		{
			LCD_sendCommand(LCD_CLEAR_COMMAND);
			LCD_moveCursor(0, 0);
			LCD_displayStrig("Door is Open");
			internal_ISR_flag_Door2=1;
			external_ISR_flag_Door+=2;
		}
	}
	else if(18<=tick_door && tick_door<33)
	{
		if(!internal_ISR_flag_Door3)
		{
			LCD_sendCommand(LCD_CLEAR_COMMAND);
			LCD_moveCursor(0, 0);
			LCD_displayStrig("Door is Locking");
			internal_ISR_flag_Door3=1;
			external_ISR_flag_Door+=2;
		}
	}
	else
	{
		tick_door=0;
		internal_ISR_flag_Door1=0;
		internal_ISR_flag_Door2=0;
		internal_ISR_flag_Door3=0;
		external_ISR_flag_Door++;
		TIMER1_deinit();
	}
}


uint8 tick=0;
void FunToTimer_3Errors()
{
	tick++;
	if(!internal_ISR_flag)
	{
		LCD_sendCommand(LCD_CLEAR_COMMAND);
		LCD_moveCursor(0, 0);
		LCD_displayStrig("ErrorErrorError");
		internal_ISR_flag=1;
		external_ISR_flag+=2;
	}
	if(tick==60)
	{
		internal_ISR_flag=0;
		external_ISR_flag++;
		TIMER1_deinit();
	}
}

/*******************************************************************************
 *                         Functions Used In Main                              *
 *******************************************************************************/

void GetThePassword(uint8 arr[])
{
	for(int i=0;i<5;i++)
	{
		arr[i]=KEYPAD_getPressedKey();
		LCD_displayCharacter('*');
		_delay_ms(400);
	}
	arr[5]='#';
	while(KEYPAD_getPressedKey()!=ENTER_KEY);
}

void GetTheFirstAndTheSecondPasswords()
{
	LCD_sendCommand(LCD_CLEAR_COMMAND);
	LCD_moveCursor(0, 0);
	LCD_displayStrig("plz enter pass: ");
	LCD_moveCursor(1, 0);
	GetThePassword(First_Password);
	LCD_sendCommand(LCD_CLEAR_COMMAND);
	LCD_moveCursor(0, 0);
	LCD_displayStrig("plz re-enter the");
	LCD_moveCursor(1, 0);
	LCD_displayStrig("same pass:");
	GetThePassword(Second_Password);
}

void EnterThePassword()
{
	LCD_sendCommand(LCD_CLEAR_COMMAND);
	LCD_moveCursor(0, 0);
	LCD_displayStrig("plz enter pass: ");
	LCD_moveCursor(1, 0);
	GetThePassword(Operating_Password);
}

void EnterTheOldPassword()
{
	LCD_sendCommand(LCD_CLEAR_COMMAND);
	LCD_moveCursor(0, 0);
	LCD_displayStrig("plz the pass: ");
	LCD_moveCursor(1, 0);
	GetThePassword(Operating_Password);
}

void MainOptions()
{
	LCD_sendCommand(LCD_CLEAR_COMMAND);
	LCD_moveCursor(0, 0);
	LCD_displayStrig("+ : Open Door");
	LCD_moveCursor(1, 0);
	LCD_displayStrig("- : Change Pass");
}

void DisplayRight()
{
	LCD_sendCommand(LCD_CLEAR_COMMAND);
	LCD_moveCursor(0, 0);
	LCD_displayStrig("Right");
	_delay_ms(1000);
}

void DisplayWrong()
{
	LCD_sendCommand(LCD_CLEAR_COMMAND);
	LCD_moveCursor(0, 0);
	LCD_displayStrig("Wrong");
	_delay_ms(1000);
}

void OpenDoor()
{
	SET_BIT(SREG,7);

	Timer1_ConfigType configs={0,31250,F_CPU_256,COMPARE};
	TIMER1_init(&configs);

	TIMER1_setCallBack(FunToTimer_Door);
	while( (external_ISR_flag_Door%2) != 0);
}

void ChangeThePassword()
{
	while(1)
	{
		GetTheFirstAndTheSecondPasswords();

		while(UART_receiveByte() != READY_TO_RECEIVE);
		UART_sendString(First_Password);

		while(UART_receiveByte() != READY_TO_RECEIVE);
		UART_sendString(Second_Password);

		UART_sendByte(READY_TO_RECEIVE);
		feedback_ChangePass=UART_receiveByte();

		if(feedback_ChangePass==MATCHED)
		{
			break;
		}
		else if(feedback_ChangePass==UNMATCHED)
		{
			continue;
		}
	}
}

void Error3Times()
{
	SET_BIT(SREG,7);

	Timer1_ConfigType configs={0,31250,F_CPU_256,COMPARE};
	TIMER1_init(&configs);

	TIMER1_setCallBack(FunToTimer_3Errors);

	while( (external_ISR_flag%2) != 0);
	wrong_counter=0;
}

/*******************************************************************************
 *                                Main Functions                               *
 *******************************************************************************/

int main(void)
{
	UART_ConfigType configs={Bit_8,Disabled,OneStopBit,9600};	/* The configurations struct of the UART */
	UART_init(&configs);										/* Initialize the UART */
	LCD_init();													/* Initialize the LCD */


	while(1)
	{
		GetTheFirstAndTheSecondPasswords(); 			/* Get the first and the second passowords */

		while(UART_receiveByte() != READY_TO_RECEIVE);  /* Wait till the C_ECU send that he is ready */
		UART_sendString(First_Password); 				/* Send the first Password */

		while(UART_receiveByte() != READY_TO_RECEIVE);  /* Wait till the C_ECU send that he is ready */
		UART_sendString(Second_Password); 				/* Send the first Password */

		UART_sendByte(READY_TO_RECEIVE); 				/* Send to the C_ECU that you are ready to receive */


		if(UART_receiveByte() == MATCHED) 				/* If the two passwords are matched */
		{
			while(2)
			{
				MainOptions();							/* Display the Main Options */

				if(KEYPAD_getPressedKey() == '+')
				{
					operation=OPEN_DOOR;
				}
				if(KEYPAD_getPressedKey() == '-')
				{
					operation=CHANGE_PASSWORD;
				}
				while(3)
				{
					EnterTheOldPassword();								/* Ask the user to enter the Password */

					while(UART_receiveByte() != READY_TO_RECEIVE);		/* Wait till the C_ECU send that he is ready */
					UART_sendString(Operating_Password);				/* Send the password to the C_ECU */

					UART_sendByte(READY_TO_RECEIVE);					/* Send to the C_ECU that you are ready to receive */
					feedback_ChangePass=UART_receiveByte();				/* Receive the feedback */

					if(feedback_ChangePass == MATCHED)					/* If Matched */
					{
						DisplayRight();

						if(operation==OPEN_DOOR)								/* If the user choose to Open the Door */
						{
							while(UART_receiveByte() != READY_TO_RECEIVE);		/* Wait till the C_ECU send that he is ready */
							UART_sendByte(OPEN_DOOR);							/* Send the operation to the C_ECU */
							operation=0;										/* Clear the operation to the coming choice */
							OpenDoor();
							break;												/* Break the while(3) loop to display the main options while(3) */
						}
						else if(operation==CHANGE_PASSWORD)						/* If the user choose to Open the Door */
						{
							while(UART_receiveByte() != READY_TO_RECEIVE);		/* Wait till the C_ECU send that he is ready */
							UART_sendByte(CHANGE_PASSWORD);						/* Send the operation to the C_ECU */
							operation=0;										/* Clear the operation to the coming choice */

							ChangeThePassword();								/* Show to the user here to enter new password */
							break;												/* Break the while(3) loop to display the main options while(3) */
						}

					}
					else if(feedback_ChangePass == UNMATCHED)					/* If the entered password was not right */
					{

						DisplayWrong();					/* Display wrong to the user */
						wrong_counter++;				/* Increase the counter of the wrong tries */
						if(wrong_counter>2)				/* When the wrong counter exceeds 3 times */
						{
							Error3Times();				/* Operate the 3 times error routine */
							break;						/* Break the while(3) loop to display the main options while(3) */
						}
					}
				}
			}
		}
		else										/* If initially the entered two passwords were not matched */
		{
			LCD_sendCommand(LCD_CLEAR_COMMAND);   	/* Clear the LCD */
			LCD_moveCursor(0, 0);
			LCD_displayStrig("NO MATCHED");		    /* Display NO MATCHED */
			_delay_ms(1000);

			for(uint8 i=0;i<=4;i++)
			{
				First_Password[i]=0;				/* Clear the two passwords */
				Second_Password[i]=0;
			}
		}
	}

}

