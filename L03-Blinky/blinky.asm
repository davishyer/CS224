;*******************************************************************************
;   CS 124 Lab 1 - blinky.asm: Software Toggle P1.0
;
;   Description: Toggle P1.0 by xor'ing P1.0 inside of a software loop.
;
;             MSP430G5223
;             -----------
;            |       P1.0|-->LED1-RED LED
;            |       P1.3|<--S2
;            |       P1.6|-->LED2-GREEN LED
;
;*******************************************************************************
           .cdecls C,"msp430.h"             ; MSP430
COUNT	   .equ		1024						; delay count

;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
;------------------------------------------------------------------------------
start:      mov.w   #0x0280,SP              ; 2 //init stack pointer
            mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; 5 //stop WDT
            bis.b   #0x40,&P1DIR            ; 4 //set P1.0 as output

mainloop:   								; LOOP TO TURN LIGHT ON
			bis.b   #0x40,&P1OUT            ; 4 //turn LED on
			mov.w 	#COUNT,r15				; 2 //use R15 as delay counter


delayloop:  								; LOOP TO DELAY BLINK IN LIGHT
			sub.w   #1,r15                  ; 1 //subtract 1 from R15
              jnz   delayloop               ; 2 //if R15 not equal to 0 loop back
            bic.b   #0x40,&P1OUT			; 4 //turn LED off
			mov.w 	#0x64,r15				; 2 //use R15 as delay counter
			mov.w	#0, r14					; 1 //use R14 as delay counter


delayloop2: 								; LOOP TO DELAY BETWEEN BLINKS
			sub.w	#1,r14					; 1 //subtract 1 from R14
			  jnz	delayloop2				; 2 //if R14 not equal to 0 loop back
			mov.w	#0x8FFF,r14				; 2 //sets R14 = 0x8FFF
			sub.w   #1,r15             		; 1 //subtract 1 from R15
              jnz   delayloop2              ; 2 //if R15 not equal to 0 loop back
            jmp     mainloop                ; 2 //jump back to beginning of loop

            								; 1116386 MCLK
            								; 1559819.46 CPI
            								; .7157 MIPS


;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   start                   ; start address
            .end
