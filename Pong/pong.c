//	pong.c	04/04/2014
//******************************************************************************
//  Pong
//
//	Davis Hyer
//	This code is my own work
//
//  Description:
//
//	Write a C program to play a 2-player game of Pong, a two-dimensional sports
//	game which simulates table tennis. Use the potentiometers to control the
//	paddles moving vertically on each side of the screen. The paddles and walls
//	reflect a ball back and forth at the correct speed and reflection angle.
//	A point is scored when one side fails to return the ball to the other side.
//	Show the score in real time on the LCD display.
//
//	Divide the paddle into segments to change the ball's angle of return.
//	For example, the center segments return the ball a 90° angle in relation to
//	the paddle, while the outer segments return the ball at smaller angles.
//	Make the ball accelerate the more it is returned back and forth between
//	paddles; missing the ball resets the speed.
//
//  Author:		Paul Roper, Brigham Young University
//  Revisions:	March 2010	Original code, split into files
//              08/05/2013	updated
//	            08/08/2013	dx,dy signed values
//              04/02/2014  pongInterrupts.c, NEW_GAME
//
//  Built with Code Composer Studio Version: 5.3.0.00090
//*******************************************************************************
//******************************************************************************
//
#include "msp430x22x4.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "pong.h"

//------------------------------------------------------------------------------
// pong global variables
volatile uint16 sys_event;				// pending events
extern BALL* ball;						// game ball object
extern PADDLE* rightPaddle;				// right paddle object
extern PADDLE* leftPaddle;

//------------------------------------------------------------------------------
//
void main(void)
{
	ERROR2(RBX430_init(_12MHZ));		// init board to 12 MHz
	ERROR2(lcd_init());					// init LCD
	ERROR2(port1_init());				// init switches
	ERROR2(ADC_init());					// init ADC
	ERROR2(watchdog_init());			// init watchdog
	ERROR2(timerA_init());				// init TimerA
	ERROR2(timerB_init());				// init TimerB
	__bis_SR_register(GIE);				// enable interrupts
	sys_event = NEW_GAME;				// start w/new game

	//-----------------------------------------------------------
	//	play forever
	while (1)
	{
		//-----------------------------------------------------------
		//	event service routine loop
		//-----------------------------------------------------------
		while (1)
		{
			// disable interrupts while checking sys_event
			_disable_interrupts();

			// if event pending, enable interrupts
			if (sys_event) _enable_interrupt();

			// else enable interrupts and goto sleep
			else __bis_SR_register(LPM0_bits | GIE);

			//-------------------------------------------------------
			//	I'm AWAKE!!!  What needs service?
			if (sys_event & MOVE_BALL)				// timer A event
			{
				sys_event &= ~MOVE_BALL;			// clear TimerA event
				MOVE_BALL_event(ball);				// update ball position
			}
			else if (sys_event & ADC_READ)			// read ADC event
			{
				sys_event &= ~ADC_READ;				// clear ADC event
				ADC_READ_event(rightPaddle);		// process ADC read
				ADC_READ_event(leftPaddle);
			}
			else if (sys_event & LCD_UPDATE)
			{
				sys_event &= ~LCD_UPDATE;
				LCD_UPDATE_event();
			}
			else if (sys_event & NEW_RALLY)
			{
				sys_event &= ~NEW_RALLY;
				NEW_RALLY_event();
			}
			else if (sys_event & NEW_GAME)			// new game event
			{
				sys_event &= ~NEW_GAME;				// clear new game event
				NEW_GAME_event();					// process new game
			}
			else if (sys_event & START_GAME)
			{
				sys_event &= ~START_GAME;			// clear start game event
				START_GAME_event();					// process start game
			}
			else if (sys_event & SWITCH_1)
			{
				sys_event &= ~SWITCH_1;				// clear switch 1 event
				SWITCH_1_event();					// process switch 1
			}
			else if (sys_event & MISSED_BALL)
			{
				sys_event &= ~MISSED_BALL;
				MISSED_BALL_event();
			}
			else if (sys_event & END_GAME)
			{
				sys_event &= ~END_GAME;
				END_GAME_event();
			}

		}
	}
} // end main
