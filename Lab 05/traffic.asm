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
;   **TX/LED_5 (G)<--|P3.4 (UCA0TXD)     (TB1) P4.4|-->LCD_BL
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

            bic.b	#0x0F,&P1SEL		; select GPIO
            bic.b	#0x0F,&P1DIR		; configure P1.0-3 as Inputs
            bis.b	#0x0F,&P1OUT		; use pull-ups
            bis.b	#0x0F,&P1REN		; enable pull-ups
            bis.b	#0x0F,&P1IES		; trigger on high to low transition
            bis.b	#0x0F,&P1IE			; P1.0-3 interrupt enabled
            bic.b	#0x0F,&P1IFG		; P1.0-3 IFG cleared

COUNT       .equ    0x8FFF

;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
            .retain                         ; Override ELF conditional linking
;-------------------------------------------------------------------------------
start:      mov.w   #__STACK_END,SP         ; init stack pointer
            mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT
            bis.b   #0x4F,&P4DIR            ; set P4 as output

mainloop:   GREEN_ON						; turn Green LED on
			mov.w	#200,r14				; set delay for 20 seconds
			call	#outer					; call delay loop
			xor.b	#0x05,&P4OUT			; turn off Green and on Yellow
			mov.w	#50,r14					; set delay for 5 seconds
			call	#outer					; call delay loop
			bic.b	#0x06,&P4OUT			; turn off Orange and Yellow
			xor.b	#0x08,&P4OUT			; turn on Red LED
			mov.w	#50,r14					; set delay for 5 seconds
			call	#outer					; call delay loop
			RED_OFF							; turn Red LED off
			 jmp	mainloop				; repeat cycle

outer:		call	#delay					; call 1/10 second delay subroutine
			dec.w	r14						; delay for desired time
			 jne	outer					; n
			ret								; y, return next instruction

delay:		push.w	r15						; callee-save
			mov.w	#COUNT,r15				; start 1/10 second delay subroutine

delayloop:	dec.w	r15                  	; delay over?
              jne   delayloop	            ; n
            pop.w	r15						; y, restore r15
            ret								; return to outer loop

P1_ISR:		bic.b	#0x0F,&P1IFG			; clear P1.0-3 Interrupt Flag
			bis.b	#0x02,&P4OUT			; turn on Orange LED
			reti





;-------------------------------------------------------------------------------
;           Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect 	.stack

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".int02"                ; MSP430 RESET Vector
            .word   P1_ISR                  ; start address
            .end
