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

;------------------------------------------------------------------------------
            .text                           ; beginning of executable code
;------------------------------------------------------------------------------
RESET:      mov.w   #0x0280,SP              ; init stack pointer
            mov.w   #WDTPW+WDTHOLD,&WDTCTL  ; stop WDT
            bis.b   #0x40,&P1DIR            ; set P1.6 as output

mainloop:   xor.b   #0x40,&P1OUT            ; toggle P1.0
            mov.w   #0,r15                  ; use R15 as delay counter

delayloop:  sub.w   #1,r15                  ; delay over?
              jnz   delayloop               ; n
            jmp     mainloop                ; y, toggle led

;------------------------------------------------------------------------------
;           Interrupt Vectors
;------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .word   RESET                   ; start address
            .end
