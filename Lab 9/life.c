//	life.c	03/24/2014
//******************************************************************************
//  The Game of Life
//
//	DAVIS HYER
//	THIS CODE IS MY OWN WORK
//
//  Lab Description:
//
//  The universe of the Game of Life is an infinite two-dimensional orthogonal
//  grid of square cells, each of which is in one of two states, alive or dead.
//  With each new generation, every cell interacts with its eight neighbors,
//  which are the cells horizontally, vertically, or diagonally adjacent
//  according to the following rules:
//
//  1. A live cell stays alive (survives) if it has 2 or 3 live neighbors,
//     otherwise it dies.
//  2. A dead cell comes to life (birth) if it has exactly 3 live neighbors,
//     otherwise it stays dead.
//
//  An initial set of patterns constitutes the seed of the simulation. Each
//  successive generation is created by applying the above rules simultaneously
//  to every cell in the current generation (ie. births and deaths occur
//  simultaneously.)  See http://en.wikipedia.org/wiki/Conway's_Game_of_Life
//
//  Author:    Paul Roper, Brigham Young University
//  Revisions: June 2013   Original code
//             07/12/2013  life_pr, life_cr, life_nr added
//             07/23/2013  generations/seconds added
//             07/29/2013  100 second club check
//             12/12/2013  SWITCHES, display_results, init for port1 & WD
//	           03/24/2014  init_life moved to lifelib.c, 0x80 shift mask
//	                       blinker added, 2x loops
//
//  Built with Code Composer Studio Version: 5.5.0.00090
//******************************************************************************
//  Lab hints:
//
//  The life grid (uint8 life[80][10]) is an 80 row x 80 column bit array.  A 0
//  bit is a dead cell while a 1 bit is a live cell.  The outer cells are always
//  dead.  A boolean cell value (0 or non-zero) is referenced by:
//
//         life[row][col >> 3] & (0x80 >> (col & 0x07))
//
//  Each life cell maps to a 2x2 lcd pixel.
//
//                     00       01             08       09
//  life[79][0-9]   00000000 00000000  ...  00000000 00000000 --> life_pr[0-9]
//  life[78][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0 --> life_cr[0-9]
//  life[77][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0 --> life_nr[0-9]
//  life[76][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0         |
//     ...                                                            |
//  life[75-4][0-9]   ...      ...            ...      ...            v
//     ...
//  life[03][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[02][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[01][0-9]   0xxxxxxx xxxxxxxx  ...  xxxxxxxx xxxxxxx0
//  life[00][0-9]   00000000 00000000  ...  00000000 00000000
//
//  The next generation can be made directly in the life array if the previous
//  cell values are held in the life_pr (previous row), life_cr (current row),
//  and life_nr (next row) arrays and used to count cell neighbors.
//
//  Begin each new row by moving life_cr values to life_pr, life_nr values to
//  life_cr, and loading life_nr with the row-1 life values.  Then for each
//  column, use these saved values in life_pr, life_cr, and life_nr to
//  calculate the number of cell neighbors of the current row and make changes
//  directly in the life array.
//
//  life_pr[0-9] = life_cr[0-9]
//  life_cr[0-9] = life_nr[0-9]
//  life_nr[0-9] = life[row-1][0-9]
//
//******************************************************************************
//******************************************************************************
// includes --------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include "msp430.h"
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "life.h"
#include <cctype>


// global variables ------------------------------------------------------------
extern volatile uint16 WDT_Sec_Cnt;		// WDT second counter
extern volatile uint16 seconds;			// # of seconds
extern volatile uint16 switches;		// debounced switch values


#define TEST_CELL(a1d,col) (a1d[(col) >> 3] & (0x80 >> ((col) & 0x07)) ? 1 : 0)
#define CELL_BIRTH(row, col) life[(row)][(col) >> 3] |= (0x80 >> ((col) & 0x07))
#define CELL_DRAW(row, col) lcd_point((col) << 1, (row) << 1, 7)
#define CELL_DEATH(row, col) life[(row)][(col) >> 3] &= ~(0x80 >> ((col) & 0x07))
#define CELL_DELETE(row, col) lcd_point((col) << 1, (row) << 1, 6)
#define RED_TOGGLE P4OUT ^= 0x40

uint8 life[NUM_ROWS][NUM_COLS/8];		// 80 x 80 life grid
uint8 life_prev[NUM_COLS/8];				// previous row
uint8 life_cur[NUM_COLS/8];				// current row
uint8 life_nex[NUM_COLS/8];				// next row

uint8* life_pr;
uint8* life_cr;
uint8* life_nr;

uint8* temp;

//------------------------------------------------------------------------------
//	draw RLE pattern -----------------------------------------------------------
void draw_rle_pattern(int row, int col, const uint8* object)
{
	int number;
	int j;
	int c;
	c = col;
	number = 0;
	while(*object && (*object != 'y'))
	{
		*object++;
	}
	while(*object && !isdigit(*object))
	{
		*object++;
	}

	while(isdigit(*object))
	{
		number = number * 10 + (*object - '0');
		*object++;
	}

	row += number;

	while(*object && *object != '\n')
	{
		*object++;
	}

	*object++;
	number = 1;

	while(*object && (*object != '!'))
	{
		if(isdigit(*object))
		{
			number = 0;
			while(isdigit(*object))
			{
				number = number * 10 + (*object - '0');
				*object++;
			}
		}

		else if(*object == 'b')
		{
			for(j = number; j > 0; j--)
			{
				CELL_DEATH(row, col);
				CELL_DELETE(row, col);
				col++;
			}

			number = 1;
			*object++;
		}

		else if(*object == 'o')
		{
			for(j = number; j > 0; j--)
			{
				CELL_BIRTH(row, col);
				CELL_DRAW(row, col);
				col++;
			}

			number = 1;
			*object++;
		}

		else if(*object == '$')
		{
			for(j = number; j > 0; j--)
			{
				row--;
			}

			col = c;
			number = 1;
			*object++;
		}
	}
	return;
} // end draw_rle_pattern


//------------------------------------------------------------------------------
// main ------------------------------------------------------------------------
void main(void)
{
	RBX430_init(_16MHZ);				// init board
	ERROR2(lcd_init());					// init LCD
	//lcd_volume(376);					// increase LCD brightness
	watchdog_init();					// init watchdog
	port1_init();						// init P1.0-3 switches
	__bis_SR_register(GIE);				// enable interrupts

	lcd_clear();
	memset(life, 0, sizeof(life));		// clear life array
	lcd_backlight(ON);
	lcd_wordImage(life_image, (HD_X_MAX - 126) / 2, 50, 1);
	lcd_cursor(10, 20);
	printf("\b\tPress Any Key");
	switches = 0;						// clear switches flag

	life_pr = life_prev;
	life_cr = life_cur;
	life_nr = life_nex;

	while (!switches);					// wait for any switch

	while (1)							// new pattern seed
	{

		uint16 generation;				// generation counter
		uint16 row, col;

		WDT_Sec_Cnt = WDT_1SEC_CNT;		// reset WD 1 second counter
		seconds = 0;					// clear second counter
		generation = 0;					// start generation counter
		int loop;
		int neighbors;
		int left;
		int middle;
		int current;
		int right;
		loop = 1;
		memset(life, 0, sizeof(life));				// clear life array
		memset(life_pr, 0, 10 * sizeof(uint8));		// clear slider
		memset(life_cr, 0, 10 * sizeof(uint8));		// clear slider
		memset(life_nr, 0, 10 * sizeof(uint8));		// clear slider
		init_life(switches);						// load seed based on switch
		switches = 0;								// reset switches



		while (loop)								// next generation
		{
			RED_TOGGLE;
			memcpy(life_pr, life[79], 10 * sizeof(uint8));
			memcpy(life_cr, life[78], 10 * sizeof(uint8));
			memcpy(life_nr, life[77], 10 * sizeof(uint8));
			// for each life row (78 down to 1)
			for (row = NUM_ROWS-2; row > 0; row--)
			{
				left = 0;
				neighbors = 0;
				current = TEST_CELL(life_cr, 1);
				right = TEST_CELL(life_pr, 2) + TEST_CELL(life_cr, 2) + TEST_CELL(life_nr, 2);
				middle = TEST_CELL(life_pr, 1) + current + TEST_CELL(life_nr, 1);

				// for each life column (78 down to 1)
				for (col = 1; col < 79; col++)
				{
					neighbors = left + (middle - current) + right;

					//neighbors += (TEST_CELL(life_pr, (col - 1)) + TEST_CELL(life_pr, col) + TEST_CELL(life_pr, (col + 1)));		// add number of neighbors on row above
					//neighbors += (TEST_CELL(life_cr, (col - 1)) + TEST_CELL(life_cr, (col + 1)));												// add number of neighbors on current row
					//neighbors += (TEST_CELL(life_nr, (col - 1)) + TEST_CELL(life_nr, col) + TEST_CELL(life_nr, (col + 1)));		// add number of neighbors on row below

					if(current == 1)												// if the cell is currently alive
					{
						if(neighbors == 2 || neighbors == 3)							// the cell has 2 or 3 neighbors
						{
																						// do nothing, the cell remains alive
						}

						else															// the cell doesn't have necessary neighbors
						{
							CELL_DEATH(row, col);										// clear cell bit in life array
							CELL_DELETE(row, col);										// clear LCD 2x2 pixel point
						}
					}

					else																// the cell is currently dead
					{
						if(neighbors == 3)												// the cell has 3 live neighbors
						{
							CELL_BIRTH(row, col);										// set cell bit in life array
							CELL_DRAW(row, col);										// set LCD 2x2 pixel point
						}
					}

					neighbors = 0;														// reset neighbors
					left = middle;
					current = TEST_CELL(life_cr, col + 1);
					middle = right;
					right = TEST_CELL(life_pr, col + 2) + TEST_CELL(life_cr, col + 2) + TEST_CELL(life_nr, col + 2);
				}

				temp = life_pr;
				life_pr = life_cr;
				life_cr = life_nr;
				life_nr = temp;
				//memcpy(life_pr, life_cr, 10 * sizeof(uint8));			// sets next row
				//memcpy(life_cr, life_nr, 10 * sizeof(uint8));			// sets next row
				memcpy(life_nr, life[row - 2], 10 * sizeof(uint8));			// sets next row


			}

			// display life generation and generations/second on LCD
			if (display_results(++generation)) break;
			if(switches)
			{
				loop = 0;					// when a switch is pressed, exit the while loop
			}
		}

	}
} // end main()
