;*******************************************************************************
;   Lab 5b - traffic.asm
;
;   Description:  1. Turn the large green LED and small red LED on and
;                    delay 20 seconds while checking for orange LED.
;                    (If orange LED is on and 10 seconds has expired, immediately
;                    skip to next step.)
;                 2. Turn large green LED off and yellow LED on for 5 seconds.
;                 3. Turn yellow LED off and large red LED on.
;                 4. If orange LED is on, turn small red LED off and small green
;                    LED on.  After 5 seconds, toggle small green LED on and off
;                    for 6 seconds at 1 second intervals.  Finish by toggling
;                    small green LED on and off for 4 seconds at 1/5 second
;                    intervals.
;                    Else, turn large red LED on for 5 seconds.
;                 5. Repeat the stoplight cycle.
;
;   I certify this to be my source code and not obtained from any student, past
;   or current.
;
;*******************************************************************************
;                            MSP430F2274
;                  .-----------------------------.
;            SW1-->|P1.0^                    P2.0|<->LCD_DB0
;            SW2-->|P1.1^                    P2.1|<->LCD_DB1
;            SW3-->|P1.2^                    P2.2|<->LCD_DB2
;            SW4-->|P1.3^                    P2.3|<->LCD_DB3
;       ADXL_INT-->|P1.4                     P2.4|<->LCD_DB4
;        AUX INT-->|P1.5                     P2.5|<->LCD_DB5
;        SERVO_1<--|P1.6 (TA1)               P2.6|<->LCD_DB6
;        SERVO_2<--|P1.7 (TA2)               P2.7|<->LCD_DB7
;                  |                             |
;         LCD_A0<--|P3.0                     P4.0|-->LED_1 (Green)
;        i2c_SDA<->|P3.1 (UCB0SDA)     (TB1) P4.1|-->LED_2 (Orange) / SERVO_3
;        i2c_SCL<--|P3.2 (UCB0SCL)     (TB2) P4.2|-->LED_3 (Yellow) / SERVO_4
;         LCD_RW<--|P3.3                     P4.3|-->LED_4 (Red)
;   ***TX/LED_5 (G)<--|P3.4 (UCA0TXD)     (TB1) P4.4|-->LCD_BL
;             RX-->|P3.5 (UCA0RXD)     (TB2) P4.5|-->SPEAKER
;           RPOT-->|P3.6 (A6)          (A15) P4.6|-->LED 6 (R)
;           LPOT-->|P3.7 (A7)                P4.7|-->LCD_E
;                  '-----------------------------'
;
;*******************************************************************************
;*******************************************************************************
            .cdecls  C,LIST,"msp430.h"      ; MSP430

            .asg    "bis.b #0x08,&P4OUT",RED_ON
            .asg    "bic.b #0x08,&P4OUT",RED_OFF
            .asg    "xor.b #0x08,&P4OUT",RED_TOGGLE
            .asg    "bit.b #0x08,&P4OUT",RED_TEST

            .asg    "bis.b #0x01,&P4OUT",GREEN_ON
            .asg    "bic.b #0x01,&P4OUT",GREEN_OFF
            .asg	"xor.b #0x01,&P4OUT",GREEN_TOGGLE
            .asg    "bit.b #0x01,&P4OUT",GREEN_TEST

COUNT       .equ    0x8FFF
AAA			.equ	200
AAB			.equ	50
AAC			.equ	60
AAD			.equ	40
AAE			.equ	10
AAF			.equ	2

;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
            .retain                         ; Override ELF conditional linking
;-------------------------------------------------------------------------------
start:      mov.w   #__STACK_END,SP         ; init stack pointer
            mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT
            bis.b   #0x4F,&P4DIR            ; set P4 as output
            bis.b	#0x10,&P3DIR			; set P3.4 as output
            bic.b	#0x0F,&P1DIR			; set P1.0-3 as input
            bis.b	#0x0F,&P1OUT			; select pull-up
            bis.b	#0x0F,&P1REN			; enable pull-ups on P1.0-3

mainloop:   bic.b	#0x0D,&P4OUT			; clear all P4 outputs
			bic.b	#0x10,&P3OUT			; clear P3.4
			GREEN_ON						; turn Green LED on
			bis.b	#0x40,&P4OUT			; turn on small red LED
			mov.w	#AAA,r14				; set delay for 20 seconds
			call	#outer					; call delay loop
			xor.b	#0x05,&P4OUT			; turn off Green and on Yellow
			mov.w	#AAB,r14				; set delay for 5 seconds
			call	#outer					; call delay loop
			mov.b	&P4OUT,r13				; check for pedestrian cycle
			xor.b	#0x02,r13				; flip bit in P4.1
			and.b	#0x02,r13				; check if Orange LED is on
			 jeq	ped						; y, jump to pedestrian phase
			bic.b	#0x06,&P4OUT			; turn off Orange and Yellow
			xor.b	#0x08,&P4OUT			; turn on Red LED
			mov.w	#AAB,r14				; set delay for 5 seconds
			call	#outer					; call delay loop
			RED_OFF							; turn Red LED off
			 jmp	mainloop				; repeat cycle

outer:		call	#delay					; call 1/10 second delay subroutine
			dec.w	r14						; delay for desired time
			 jne	outer					; n
			ret								; y, return next instruction

delay:		mov.b	&P1IN,r13				; sample buttons
			and.b	#0x0F,r13				; mask least significant nybble
			xor.b	#0x0F,r13				; button pressed?
			 jeq	off						; no
			bis.b	#0x02,P4OUT				; y, turn on Orange LED

off:		push.w	r15						; callee-save
			mov.w	#COUNT,r15				; start 1/10 second delay subroutine

delayloop:  dec.w	r15                  	; delay over?
              jne   delayloop	            ; n
            pop.w	r15						; y, restore r15
            ret								; return to outer loop

ped:		bic.b	#0x06,&P4OUT			; turn off Orange and Yellow
			xor.b	#0x48,&P4OUT			; turn on Red LED, small red LED off
			xor.b	#0x10,&P3OUT			; turn on small green LED
			mov.w	#AAB,r14				; set delay for 5 seconds
			call	#outer					; call delay loop
			mov.w	#AAC,r14				; set delay for 6 seconds
			mov.w	#AAE,r12				; set duration for 1 second
			call	#pedouter				; call pedestrian delay loop
			mov.w	#AAD,r14				; set delay for 4 seconds
			mov.w	#AAF,r12				; set duration for 1/5 second
			call	#pedouter2				; call pedestrian2 delay loop
			 jmp 	mainloop				; repeat cycle

pedouter:	call	#delay					; call 1/10 second delay subroutine
			dec.w	r12						; duration check
			 jeq	pedlight				; y, flip light

pedouter_:	dec.w	r14						; delay over?
			 jne	pedouter				; n
			ret								; y, return to pedestrian cycle

pedlight:	xor.b	#0x10,&P3OUT			; toggle small green LED
			mov.w	#AAE,r12				; reset duration for 1 second
			 jmp	pedouter_				; return to pedestrian delay

pedouter2:	call	#delay					; call 1/10 second delay subroutine
			dec.w	r12						; duration check
			 jeq	pedlight2				; y, flip light

pedouter_2:	dec.w	r14						; delay over?
			 jne	pedouter2				; n
			ret								; y, return to pedestrian cycle

pedlight2:	xor.b	#0x10,&P3OUT			; toggle small green LED
			mov.w	#AAF,r12				; reset duration for 1/5 second
			 jmp	pedouter_2				; return to pedestrian delay



;-------------------------------------------------------------------------------
;           Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect 	.stack

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   start                   ; start address
            .end
