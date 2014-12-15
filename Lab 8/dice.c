//******************************************************************************
//  Lab 8a - Dice
//
//	Name: Davis Hyer
//	Section: 02
//	I wrote this code on my own.
//
//  Description:
//
//	"Write a dice roller C program that waits for a switch to be pressed and then
//	 displays two dice which randomly change values. The dice roll begins rapidly
//	 and progressively slows down until it stops (after approximately 3-5
//	 seconds). A congratulations ditty is played when doubles are rolled. If
//	 doubles are rolled twice in a row, output a raspberry tone. Write an
//	 assembly language function which returns a random number from 1 to 6 and
//	 call this function from your C program." 
//
//  Author:		Paul Roper, Brigham Young University
//	Revisions:	March 2010	Original code
//				03/15/2014	drawDie directly draws die
//							seconds, blink LED added
//
//  Built with Code Composer Studio Version: 5.x
//******************************************************************************
// includes
#include "msp430.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "dice.h"

// external/internal prototypes ------------------------------------------------
extern uint16 random(int low, int high);	// get random # between low and high
uint16 myMod(int n, int m);					// return (n % m)
extern volatile uint16 seconds;				// second counter


// Mod the number function -----------------------------------------------------
uint16 myMod(int n,int m)
{
	n=n%m;
	return n;
}


//------------------------------------------------------------------------------
// main ------------------------------------------------------------------------
void main(void)
{
	RBX430_init(_8MHZ);						// init board
	ERROR2(lcd_init());						// init LCD
	ERROR2(WDT_init());						// init watchdog as timer
	ERROR2(timerB_init());					// init timerB for tones
	__bis_SR_register(GIE);					// enable interrupts

	lcd_clear();							// clear LCD
	lcd_backlight(ON);						// turn on LCD backlight
	lcd_wordImage(byu4_image, 42, 110, 1);	// draw byu logo
	lcd_rectangle(15, 45, 60, 60, 3);		// draw double die frame
	lcd_rectangle(87, 45, 60, 60, 3);		// draw double die frame

	int permDie1 = 1;
	int permDie2 = 1;
	int doubles = 0;
	int secDoubles = 0;
	int delayNum = 7;
	int i = 3;
	int stop = seconds;

	while (1)								// always run
	{
		if((P1IN & 0x0f) ^ 0x0f)
		{

			lcd_backlight(ON);
			i = 3;
			while(delayNum >= 0)
			{
				uint8 myDie = random(1,6);			// get a random die (1-6)
				drawDie(myDie, 22, 51);				// draw a die
				uint8 myDie2 = random(1,6);			// get a second random die
				drawDie(myDie2, 92, 51);			// draw die with second number
				lcd_cursor(10, 15);					// position message
				lcd_printf("\b\tDice Are Rolling ");
				doTone(BEEP, BEEP_CNT);				// output BEEP						ADD BEEP BACK IN
				WDT_delay(i);						// delay
				permDie1 = myDie;
				permDie2 = myDie2;
				i = i + i;
				if((P1IN & 0x0f) ^ 0x0f)
				{
					i = 3;
					delayNum = 7;
				}
				delayNum = delayNum - 1;
			}

			doTone((BEEP / 2), BEEP_CNT);

			if(permDie1 == permDie2)
			{
				doubles = 1;
				WDT_delay(200);
				doTone(TONE_G, 5 * BEEP_CNT);
				doTone(TONE_G, 5 * BEEP_CNT);
				doTone(TONE_G, 5 * BEEP_CNT);
				doTone(TONE_Eb, 4 * BEEP_CNT);
				doTone(TONE_Bb, 1 * BEEP_CNT);
				doTone(TONE_G, 5 * BEEP_CNT);
				doTone(TONE_Eb, 4 * BEEP_CNT);
				doTone(TONE_Bb, 1 * BEEP_CNT);
				doTone(TONE_G, 5 * BEEP_CNT);

				if((doubles + secDoubles) == 2)
				{
					doTone(TONE_Cs, 7 * BEEP_CNT);
				}

				secDoubles = 1;
				doubles = 0;
			}

			else
			{
				secDoubles = 0;
				doubles = 0;
			}

			seconds = 0;
		}

		else
		{
			stop = seconds;
			if(stop == 5)
			{
				lcd_backlight(OFF);
			}
			delayNum = 7;
			drawDie(permDie1, 22, 51);				// keep the roll displayed
			drawDie(permDie2, 92, 51);				// keep the other roll displayed
			lcd_cursor(10, 15);						// position message
			lcd_printf("\b\tPress A Button");		// prompt user to roll again
		}
	}
} // end main()
