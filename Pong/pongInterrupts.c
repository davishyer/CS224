//	pongInterrupts.c	04/08/2014
//******************************************************************************

#include "msp430x22x4.h"
#include <stdlib.h>
#include "RBX430-1.h"
#include "RBX430_lcd.h"
#include "pong.h"

volatile uint16 WDT_cps_cnt;				// watchdog counters
volatile uint16 WDT_adc_cnt;
volatile uint16 WDT_lcd_cnt;
volatile uint16 WDT_debounce_cnt;			// WDT debounce counter
volatile uint16 TB0_tone_on;				// PWM tones
volatile uint16 ball_speed;
volatile uint16 seconds;

extern volatile uint16 sys_event;			// pending events
extern volatile uint16 mode;				// game mode

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//	initialize functions
//
int watchdog_init(void)
{
	// configure watchdog
	WDTCTL = WDT_CTL;						// set Watchdog interval
	IE1 |= WDTIE;							// Enable WDT interrupt
	WDT_debounce_cnt = 0;					// clear debounce count
	WDT_adc_cnt = 0;
	WDT_lcd_cnt = WDT_LCD;
	WDT_cps_cnt = WDT_CPS;
	return 0;
} // end watchdog_init


int timerA_init(void)
{
	// configure TimerA to move the ball
	TAR = 0;								// reset timer
	TACCR0 = 0;								// interrupt rate
	TACTL = TASSEL_2|ID_3|MC_1|TAIE;		// SMCLK/8, up mode, interrupt enable
	return 0;
} // end timerA_init


int timerB_init(void)
{
	// configure TimerB h/w PWM for speaker
	P4SEL |= 0x20;							// P4.5 TB2 output
	TBR = 0;								// reset timer B
	TBCTL = TBSSEL_2 | ID_0 | MC_1;			// SMCLK, /1, UP (no interrupts)
	TBCCTL2 = OUTMOD_3;						// TB2 = set/reset
	return 0;
} // end timerB_init


int port1_init(void)
{
	// configure P1 switches and ADXL345 INT1 for interrupt
	P1SEL &= ~0x0f;						// select GPIO
	P1DIR &= ~0x0f;						// Configure P1.0-3 as Inputs
	P1OUT |= 0x0f;						// use pull-ups
	P1IES |= 0x0f;						// high to low transition
	P1REN |= 0x0f;						// Enable pull-ups
	P1IE |= 0x0f;						// P1.0-3 interrupt enabled
	P1IFG &= ~0x0f;						// P1.0-3 IFG cleared
	WDT_debounce_cnt = 0;				// clear debounce count
	return 0;
} // end port1_init


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//	Watchdog Timer ISR
//
#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void)
{
	if (--WDT_cps_cnt == 0)				// 1 second?
	{
		LED_GREEN_TOGGLE;				// toggle green LED
		WDT_cps_cnt = WDT_CPS;			// reset counter
		++seconds;						// increment second counter
	}

	// check for switch debounce
	if (WDT_debounce_cnt && (--WDT_debounce_cnt == 0))
	{
		sys_event |= (P1IN ^ 0x0f) & 0x0f;
	}

	// decrement tone counter - turn off tone when 0
	if (TB0_tone_on && (--TB0_tone_on == 0)) TBCCR0 = 0;

	// decrement adc event counter - signal to read adc
	if (WDT_adc_cnt && (--WDT_adc_cnt == 0))
	{
		WDT_adc_cnt = WDT_ADC;
		if (mode == PLAY)
			sys_event |= ADC_READ;		// signal ADC event
	}

	// decrement update lcd event counter - signal to update lcd
	if (WDT_lcd_cnt && (--WDT_lcd_cnt == 0))
	{
		WDT_lcd_cnt = WDT_LCD;
		if (mode == PLAY)
		{
			sys_event |= LCD_UPDATE;	// signal LCD_UPDATE event
		}
	}

	// wake up processor if there are any events pending
	if (sys_event) __bic_SR_register_on_exit(LPM0_bits);
	return;
} // end WDT_ISR


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Timer A1 interrupt service routine
//
#pragma vector = TIMERA1_VECTOR
__interrupt void TIMERA1_ISR(void)
{
	TACTL &= ~TAIFG;					// acknowledge interrupt
	TACCR0 = ball_speed;				// adjust ball speed
	if (mode == PLAY)
	{
		sys_event |= MOVE_BALL;			// signal move ball event
		__bic_SR_register_on_exit(LPM0_bits);
	}
	return;
} // end TIMERA1_VECTOR


//------------------------------------------------------------------------------
// Port 1 interrupt service routine
//
#pragma vector=PORT1_VECTOR
__interrupt void Port_1_ISR(void)
{
	P1IFG &= ~0x0f;						// P1.0-3 IFG cleared
	WDT_debounce_cnt = DEBOUNCE_CNT;	// enable debounce
	return;
} // end Port_1_ISR


//------------------------------------------------------------------------------
// un-initialized MSP430F2274 interrupt vectors
#pragma vector=PORT2_VECTOR,TIMER0_A0_VECTOR,NMI_VECTOR,	\
		USCIAB0RX_VECTOR,USCIAB0TX_VECTOR,TIMERB0_VECTOR,TIMERB1_VECTOR
__interrupt void ISR_trap(void)
{
	ERROR2(SYS_ERR_ISR);				// unrecognized event
	WDTCTL = 0;							// Write to WDT with a wrong password
} // end ISR_trap
