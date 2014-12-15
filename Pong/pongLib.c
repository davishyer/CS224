//	pongLib.c	04/04/2014
//******************************************************************************
//******************************************************************************
//	ALL THESE FUNCTIONS SHOULD BE WORKING PROPERLY - NO CHANGE NECESSARY!
//******************************************************************************
//******************************************************************************
//
#include "msp430x22x4.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "pong.h"

extern volatile uint16 sys_event;				// pending events

//*******************************************************************************
//	draw paddle
//
//	returns y position of the middle of paddle
//
//							XXX  y+7	reflect 22.5
//							XXX  y+6	reflect 22.5
//							XXX  y+5	reflect 45
//							XXX  y+4	reflect 45
//							XXX  y+3	reflect 45
//							XXX  y+2	reflect 45
//							XXX  y+1	reflect 45
//		myPaddle->y ---->	XXX  y		reflect 90
//							XXX  y-1	reflect 45
//							XXX  y-2	reflect 45
//							XXX  y-3	reflect 45
//							XXX  y-4	reflect 45
//							XXX  y-5	reflect 45
//							XXX  y-6	reflect 22.5
//							XXX  y-7	reflect 22.5
//
#define MIDDLE	7
//
int drawPaddle(PADDLE* myPaddle)
{
	if ((myPaddle->y - MIDDLE) > 140) myPaddle->y = 140 + MIDDLE;
	if ((myPaddle->y - MIDDLE) < 3) myPaddle->y = 3 + MIDDLE;

	lcd_fillImage(myPaddle->x, myPaddle->old_y - MIDDLE, 3, 16, 0);
	lcd_fillImage(myPaddle->x, myPaddle->y - MIDDLE, 3, 16, 5);
	myPaddle->old_y = myPaddle->y;
	return myPaddle->y;
} // end drawPaddle


//*******************************************************************************
//	draw ball
//
//	returns y position of the middle of ball
//
//		 OOO
//		OOOOO
//		OO.OO
//		OOOOO
//		 OOO
//
#define LEFT	1
#define RIGHT	2
#define DOWN	3
#define UP		4

int drawBall(BALL* ball)
{
	int position;
	do
	{
		int x = ball->old_x;
		int y = ball->old_y;

		if (x > ball->x) position = LEFT;
		else if (x < ball->x) position = RIGHT;
		else if (y > ball->y) position = DOWN;
		else if (y < ball->y) position = UP;
		else position = 0;

		switch (position)
		{
			case 0:
			{
				lcd_point(x-1, y+2, 1);
				lcd_point(x-0, y+2, 1);
				lcd_point(x+1, y+2, 1);

				lcd_point(x-2, y+1, 1);
				lcd_point(x-1, y+1, 1);
				lcd_point(x+0, y+1, 1);
				lcd_point(x+1, y+1, 1);
				lcd_point(x+2, y+1, 1);

				lcd_point(x-2, y+0, 1);
				lcd_point(x-1, y+0, 1);
				lcd_point(x+0, y+0, 1);
				lcd_point(x+1, y+0, 1);
				lcd_point(x+2, y+0, 1);

				lcd_point(x-2, y-1, 1);
				lcd_point(x-1, y-1, 1);
				lcd_point(x+0, y-1, 1);
				lcd_point(x+1, y-1, 1);
				lcd_point(x+2, y-1, 1);

				lcd_point(x-1, y-2, 1);
				lcd_point(x-0, y-2, 1);
				lcd_point(x+1, y-2, 1);
				break;
			}

			case UP:							// up
			{
				lcd_point(x-1, y+3, 1);
				lcd_point(x-0, y+3, 1);
				lcd_point(x+1, y+3, 1);

				lcd_point(x-2, y+2, 1);
				lcd_point(x+2, y+2, 1);

				lcd_point(x-2, y-1, 0);
				lcd_point(x+2, y-1, 0);

				lcd_point(x-1, y-2, 0);
				lcd_point(x-0, y-2, 0);
				lcd_point(x+1, y-2, 0);
				ball->old_y++;
				break;
			}

			case DOWN:							// down
			{
				lcd_point(x-1, y-3, 1);
				lcd_point(x-0, y-3, 1);
				lcd_point(x+1, y-3, 1);

				lcd_point(x-2, y-2, 1);
				lcd_point(x+2, y-2, 1);

				lcd_point(x-2, y+1, 0);
				lcd_point(x+2, y+1, 0);

				lcd_point(x-1, y+2, 0);
				lcd_point(x-0, y+2, 0);
				lcd_point(x+1, y+2, 0);
				ball->old_y--;
				break;
			}

			case RIGHT:							// right
			{
				lcd_point(x+3, y-1, 1);
				lcd_point(x+3, y+0, 1);
				lcd_point(x+3, y+1, 1);

				lcd_point(x+2, y-2, 1);
				lcd_point(x+2, y+2, 1);

				lcd_point(x-1, y-2, 0);
				lcd_point(x-1, y+2, 0);

				lcd_point(x-2, y-1, 0);
				lcd_point(x-2, y+0, 0);
				lcd_point(x-2, y+1, 0);
				ball->old_x++;
				break;
			}

			case LEFT:							// left
			{
				lcd_point(x-3, y-1, 1);
				lcd_point(x-3, y+0, 1);
				lcd_point(x-3, y+1, 1);

				lcd_point(x-2, y-2, 1);
				lcd_point(x-2, y+2, 1);

				lcd_point(x+1, y-2, 0);
				lcd_point(x+1, y+2, 0);

				lcd_point(x+2, y-1, 0);
				lcd_point(x+2, y+0, 0);
				lcd_point(x+2, y+1, 0);
				ball->old_x--;
				break;
			}
		}
	} while ((ball->x != ball->old_x) || (ball->y != ball->old_y));

	return ball->y;
} // end drawBall
