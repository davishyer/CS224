//	pongEvents.c	04/04/2014
//******************************************************************************
//******************************************************************************
//	ALL THESE FUNCTIONS REQUIRE REWORK!
//******************************************************************************
//******************************************************************************
//
#include "msp430x22x4.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "pong.h"

extern volatile int16 dx, dy;
extern volatile uint16 TB0_tone_on;
extern volatile uint16 sys_event;		// pending events

extern volatile uint16 seconds;
extern volatile uint16 WDT_adc_cnt;
extern volatile uint16 ball_speed;

BALL* myBall;							// **replace with malloc'd struct**
PADDLE* myPaddle;
BALL* ball;								// game ball object
PADDLE* rightPaddle;					// right paddle object
PADDLE* leftPaddle;						// left paddle object
volatile int16 dx, dy;					// ball delta change
volatile uint16 mode;
volatile int scoreL;
volatile int scoreR;
volatile int time;
volatile int i;

extern const uint16 pong_image[];		// 2 paddle image
extern const uint8 one[];
extern const uint8 two[];
extern const uint8 three[];
extern const uint8 go[];

extern volatile uint16 WDT_adc_cnt;
extern volatile uint16 ball_speed;

//------------------------------------------------------------------------------
//	ball factory (fix by malloc'ing ball struct)
//
BALL* new_ball(int x, int y)
{
	myBall = (BALL*)malloc(sizeof(BALL));
	myBall->x = x;						// set horizontal position
	myBall->y = y;						// set vertical position
	myBall->old_x = myBall->x;			// set old values
	myBall->old_y = myBall->y;
	return myBall;
} // end init_ball


//------------------------------------------------------------------------------
//	paddle factory (fix by malloc'ing paddle struct)
//
PADDLE* new_paddle(int channel, int x)
{
	myPaddle = (PADDLE*)malloc(sizeof(PADDLE));
	myPaddle->channel = channel;			// ADC input channel
	myPaddle->x = x;						// horizontal paddle position
	myPaddle->y = ADC_read(channel) >> 2;
	myPaddle->old_y = 0;
	return myPaddle;					// return paddle
} // end init_ball


//------------------------------------------------------------------------------
//	new game event (fix)
//
void NEW_GAME_event()
{
	lcd_clear();							// clear LCD
	lcd_volume(345);						// **increase as necessary**
	lcd_backlight(ON);						// turn on LCD
	lcd_wordImage(pong_image, 25, 35, 1);
	lcd_mode(LCD_2X_FONT);					// turn 2x font on
	lcd_cursor(20, 30);						// set display coordinates
	printf("P O N G");
	lcd_mode(~LCD_2X_FONT);					// turn 2x font off
	lcd_cursor(30, 5);
	lcd_printf("PRESS ANY SWITCH");
	mode = IDLE;

	return;
} // end NEW_GAME_event


//------------------------------------------------------------------------------
//	ADC event - paddle potentiometer and draw paddle (fix)
//
void ADC_READ_event(PADDLE* paddle)
{
	int pot = 1023-ADC_read(paddle->channel);	// sample potentiometer

	// check for paddle position change
	if(paddle->channel == LEFT_POT)
	{
		abs(pot - paddle->potValue) > POT_THRESHHOLD;
		paddle->potValue = pot;			// save old value
		paddle->y = 159 - (pot / (1024.0 / 160));		// update paddle position
		drawPaddle(paddle);				// draw paddle
	}
	else
	{
		abs(pot - paddle->potValue) > POT_THRESHHOLD;
		paddle->potValue = pot;			// save old value
		paddle->y = pot / (1024.0 / 160);		// update paddle position
		drawPaddle(paddle);				// draw paddle
	}
	return;
} // end ADC_READ_event


//------------------------------------------------------------------------------
//	TimerA event - move ball (fix)
//
void MOVE_BALL_event(BALL* ball)
{
	ball->x += dx;						// increment x coordinate
	ball->y += dy;						// update y coordinate
	if ((ball->y > HD_Y_MAX-6) || (ball->y < 2))
	{
		dy = -dy;
	}
	drawBall(ball);						// ok, draw ball in new position

	if ((ball->x <= 3) || (ball->x >= HD_X_MAX-3))
	{
		dx = -dx;						// reverse ball direction
	}

	if (ball->x == (HD_X_MAX - 3))
	{
		if(ball->y <= (rightPaddle->y + 12) && ball->y >= (rightPaddle->y - 11))
		{
			BEEP;
			if(ball->y == rightPaddle->y) dy = 0;		// change angle to 90
			else if (ball->y <= (rightPaddle->y + 4) && ball->y >= (rightPaddle->y - 3))
			{
				if(dy > 0) dy = 1;						// change angle to 45
				else dy = -1;							// change angle to 45
			}
			else
			{
				if(dy > 0) dy = 2;						// change angle to 23.5
				else dy = -2;							// change angle to 23.5
			}

			ball_speed = ball_speed - 1000;				// increase the speed
			if(ball_speed == 1000)
			{
				ball_speed = 2000;
			}
		}
		else
		{
			i = 1;										// next serve will be from left
			scoreL++;									// increase left paddle's score
			ball->x = 190;
			drawBall(ball);
			sys_event |= MISSED_BALL;					// signal missed ball
		}
	}
	if (ball->x == 3)
	{

		if(ball->y <= (leftPaddle->y + 12) && ball->y >= (leftPaddle->y - 11))
		{
			BEEP;
			if(ball->y == leftPaddle->y) dy = 0;	// change angle to 90
			else if (ball->y <= (leftPaddle->y + 4) && ball->y >= (leftPaddle->y - 3))
			{
				if(dy > 0) dy = 1;					// change angle to 45
				else dy = -1;						// change angle to 45
			}
			else
			{
				if(dy > 0) dy = 2;					// change angle to 23.5
				else dy = -2;						// change angle to 23.5
			}

			ball_speed = ball_speed - 1000;			// increase the speed
			if(ball_speed == 1000)
			{
				ball_speed = 2000;
			}
		}

		else
		{
			i = -1;									// next serve will be from right
			scoreR++;								// increase right paddle's score
			ball->x = -10;
			drawBall(ball);
			sys_event |= MISSED_BALL;				// signal missed ball
		}
	}

	return;
} // end MOVE_BALL_event

//-------------------------------------------------------------------------------
//
//
void START_GAME_event()
{
	scoreL = 0;								// set left score to 0
	scoreR = 0;								// set right score to 0
	i = 1;									// initialize the direction of ball
	time = seconds;							// initialize the delay
	lcd_clear();							// clear the splash screen
	while((time + 1) != seconds);			// wait one second
	lcd_bitImage(three, 70, 70, 1);			// display 3
	while((time + 2) != seconds);			// wait one second
	lcd_bitImage(two, 70, 70, 1);			// display 2
	while((time + 3) != seconds);			// wait one second
	lcd_bitImage(one, 70, 70, 1);			// display 1
	while((time + 4) != seconds);
	lcd_bitImage(go, 70, 70, 1);
	while((time + 5) != seconds);
	lcd_clear();

	// manufacture and draw paddles
	rightPaddle = new_paddle(RIGHT_POT, 157);
	drawPaddle(rightPaddle);
	leftPaddle = new_paddle(LEFT_POT, 0);
	drawPaddle(leftPaddle);
	WDT_adc_cnt = 1;					// start sampling the paddles

	sys_event |= NEW_RALLY;				// signal new rally

	return;
}// end START_GAME_event

//-------------------------------------------------------------------------------
//
//
void SWITCH_1_event()
{
	if(mode == IDLE) sys_event |= START_GAME;
	else sys_event |= NEW_GAME;
	return;
}// end SWITCH_1_event

//-------------------------------------------------------------------------------
//
//
void LCD_UPDATE_event()
{
	if(mode == PLAY)
	{
		lcd_cursor(40, 140);
		lcd_printf("%d", scoreL);
		lcd_cursor(120, 140);
		lcd_printf("%d", scoreR);
	}
	return;
}// end LCD_UPDATE_event

//-------------------------------------------------------------------------------
//
//
void MISSED_BALL_event()
{
	RASP;
	free(ball);
	mode = IDLE;
	if(scoreL < 5 && scoreR < 5) sys_event |= NEW_RALLY;
	else sys_event |= END_GAME;
	return;
}// end MISSED_BALL_event

//-------------------------------------------------------------------------------
//
//
void END_GAME_event()
{
	mode = PLAY;
	LCD_UPDATE_event();
	mode = IDLE;
	free(rightPaddle);
	free(leftPaddle);
	if(scoreL == 5)
	{
		lcd_cursor(40, 80);
		lcd_printf("WINNER");
	}

	else
	{
		lcd_cursor(120, 80);
		lcd_printf("WINNER");
	}

	time = seconds;
	while((time + 5) != seconds);
	sys_event |= NEW_GAME;
	return;
}// end END_GAME_event

//-------------------------------------------------------------------------------
//
//
void NEW_RALLY_event()
{
	// manufacture and draw ball
	ball = new_ball(80, 100);
	drawBall(ball);
	dx = i;								// delta x
	dy = rand() % 2 ? -1 : 1;			// delta y

	ball_speed = BALL_SPEED;
	TACCR0 = ball_speed;				// interrupt rate

	mode = PLAY;
	return;
}// end NEW_RALLY_event
