			.title	"morse.asm"
;*******************************************************************************
;     Project:  morse.asm
;      Author:  Davis Hyer

;    This code is my own work.
;
; Description:  Outputs a message in Morse Code using a LED and a transducer
;               (speaker).  The watchdog is configured as an interval timer.
;               The watchdog interrupt service routine (ISR) toggles the green
;               LED every second and pulse width modulates (PWM) the speaker
;               such that a tone is produced.
;
;  numbers--->N0$--->DASH,DASH,DASH,DASH,DASH,END      ; 0
;             N1$--->DOT,DASH,DASH,DASH,DASH,END       ; 1
;             ...
;             N9$--->DASH,DASH,DASH,DASH,DOT,END       ; 9
;
;  letters--->A$---->DOT,DASH,END                      ; A
;             B$---->DASH,DOT,DOT,DOT,END              ; B
;             ...
;             Z$---->DASH,DASH,DOT,DOT,END             ; Z
;
;	Morse code is composed of dashes and dots, or phonetically, "dits" and
;    "dahs".  There is no symbol for a space in Morse, though there are rules
;    when writing them.
;
;	1. One dash is equal to three dots
;	2. The space between parts of the letter is equal to one dot
;	3. The space between two letters is equal to three dots
;	4. The space between two words is equal to seven dots.
;
;	5 WPM = 60 sec / (5 * 50) elements = 240 milliseconds per element.
;	element = (WDT_IPS * 6 / WPM) / 5
;
;******************************************************************************
; System equates --------------------------------------------------------------
            .cdecls C,"msp430.h"            ; include c header
myCLOCK     .equ    1200000                 ; 1.2 Mhz clock
WDT_CTL     .equ    WDT_MDLY_0_5            ; WD: Timer, SMCLK, 0.5 ms
WDT_CPI     .equ    500                     ; WDT Clocks Per Interrupt (@1 Mhz)
WDT_IPS     .equ    myCLOCK/WDT_CPI         ; WDT Interrupts Per Second
STACK       .equ    0x0600                  ; top of stack

; External references ---------------------------------------------------------
            .ref    numbers                 ; codes for 0-9
            .ref    letters                 ; codes for A-Z
            .ref    DOT,DASH,END

; Morse Code equoates ---------------------------------------------------------
ELEMENT     .equ    WDT_IPS*240/1000        ; (WDT_IPS * 6 / WPM) / 5
DEBOUNCE 	.equ    10
                            
; Global variables ------------------------------------------------------------
            .bss    beep_cnt,2              ; beeper flag
            .bss    delay_cnt,2             ; delay flag
            .bss	GREEN,2					; green delay
            .bss    debounce_cnt,2       	; debounce count

; Program section -------------------------------------------------------------
            .text                           ; program section
message:    .string "HELLO CS124 WORLD"                 ; PARIS message
            .byte   0
            .align  2                       ; align on word boundary

RESET:      mov.w   #STACK,SP               ; initialize stack pointer
            mov.w   #WDT_CTL,&WDTCTL        ; set WD timer interval
            mov.b   #WDTIE,&IE1             ; enable WDT interrupt
            bis.b   #0x61,&P4DIR            ; set P4 as output (speaker, small red LED)
            bis.b	#0x10,&P3DIR			; set P3 as output (small green LED)
            clr.w   &beep_cnt               ; clear counters
            clr.w   &delay_cnt
            bis.w   #GIE,SR                 ; enable interrupts
            mov.w	#WDT_IPS,&GREEN			; initialze green delay
            bic.b   #0x0f,&P1SEL         	; RBX430-1 push buttons
         	bic.b   #0x0f,&P1DIR          	; Configure P1.0-3 as Inputs
        	bis.b   #0x0f,&P1OUT          	; pull-ups
         	bis.b   #0x0f,&P1IES          	; h to l
         	bis.b   #0x0f,&P1REN          	; enable pull-ups
         	bis.b   #0x0f,&P1IE           	; enable switch interrupts

;----------------------------------------------------------------------------
;LOADING MESSAGE AND CYCLING THROUGH EACH CHARACTER
loop:       mov.w   #message,r4          	; output DOT

lp02:		mov.b	@r4+,r5					; load first character
			cmp.b	#0x20,r5				; is this a space?
			 jeq	lpSPACE					; y
			cmp.b	#0x40,r5				; is this a letter?
			 jge	lpLET					; y
			cmp.b	#0x2F,r5				; is this a number
			 jge	lpNUM					; y
			 jmp	loop					; reload the message

lpSPACE:	call	#doSPACE				; output a space
			 jmp	lp02					; return to lp02

lpNUM:		sub.w	#0x30,r5				; set start at 0
			add.w	r5,r5
			mov.w	numbers(r5),r5			; move memory location of number to r5
			 jmp	lp03					; jump down to lp03

lpLET:		sub.w	#0x41,r5				; set to start at A
			add.w	r5,r5
			mov.w	letters(r5),r5			; move memory location of letter to r5

lp03:		mov.b	@r5+,r6					; move bit into r6
			cmp.b	#DOT,r6					; is this a dot?
			 jeq	doDOT					; y, output dot
			cmp.b	#DASH,r6				; is this a dash?
			 jeq	doDASH					; y, output dash
			cmp.b	#END,r6					; is this a end?
			 jeq	doGAP					; y, output gap
			 jmp	lp02					; get next character


;---------------------------------------------------------------------------
;FUNCTIONS FOR APPROPRIATE OUTPUT
doDOT:		push.w	r15						; callee-save
			mov.w	#ELEMENT,r15			; set delay for one unit
			xor.b	#0x40,&P4OUT			; toggle red LED
			call	#beep					; call beep function
			xor.b	#0x40,&P4OUT			; toggle red LED
			mov.w	#ELEMENT,r15			; set delay for one unit
			call	#delay					; call delay function
			pop.w	r15						; callee-save
			 jmp	lp03					; return to lp03

doDASH:		push.w	r15						; callee-save
			mov.w	#ELEMENT*3,r15			; set delay for three units
			xor.b	#0x40,&P4OUT			; toggle red LED
			call	#beep					; call beep function
			xor.b	#0x40,&P4OUT			; toggle red LED
			mov.w	#ELEMENT,r15			; set delay for one unit
			call	#delay					; call delay function
			pop.w	r15						; callee-save
			 jmp	lp03					; return to lp03

doSPACE:	push	r15						; callee-save
			mov.w	#ELEMENT*6,r15			; set delay for seven units
			call	#delay					; call delay function
			 jmp	lp03					; return to lp03

doGAP:		push	r15						; callee-save
			mov.w	#ELEMENT*2,r15			; set delay for three units
			call	#delay					; call delay function
			pop.w	r15						; callee-save
			 jmp	lp02					; return to lp03



; beep (r15) ticks subroutine -------------------------------------------------
beep:       mov.w   r15,&beep_cnt           ; start beep

beep02:     tst.w   &beep_cnt               ; beep finished?
              jne   beep02                  ; n
            ret                             ; y


; delay (r15) ticks subroutine ------------------------------------------------
delay:      mov.w   r15,&delay_cnt          ; start delay

delay02:    tst.w   &delay_cnt              ; delay done?
              jne   delay02                 ; n
            ret                             ; y


; Watchdog Timer ISR ----------------------------------------------------------
WDT_ISR:    tst.w   &beep_cnt               ; beep on?
              jeq   WDT_02                  ; n
            dec.w   &beep_cnt               ; y, decrement count
         	xor.b   #0x20,&P4OUT            ; beep using 50% PWM

WDT_02:     tst.w   &delay_cnt              ; delay?
              jeq   WDT_03                  ; n
            dec.w   &delay_cnt              ; y, decrement count

WDT_03:		dec.w	&GREEN					; time to toggle?
			 jne	WDT_04					; n
			xor.w	#0x10, &P3OUT			; toggle green LED
			mov.w	#WDT_IPS,&GREEN			; re-initialze green delay

WDT_04:		tst.w   debounce_cnt         	; debouncing?
             jeq   	WDT_10               	; n
            dec.w   debounce_cnt         	; y, decrement count, done?
             jne   	WDT_10              	; n
         	push    r15                   	; callee-save
         	mov.b   &P1IN,r15             	; read switches
         	and.b   #0x0f,r15				; mask - only look at bits we care about
         	xor.b   #0x0f,r15             	; was a button pressed?
             jeq   	WDT_05               	; n
            xor.b	#0x20,&P4DIR			; toggle speaker

WDT_05:		pop		r15       				; callee-save

WDT_10:     reti                            ; return from interrupt

; P1 ISR ----------------------------------------------------------------------
P1_ISR:    	bic.b   #0x0f,&P1IFG          	; acknowledge (put hands down)
           	mov.w   #DEBOUNCE,debounce_cnt 	; reset debounce count
           	reti


; Interrupt Vectors -----------------------------------------------------------
            .sect   ".int10"                ; Watchdog Vector
            .word   WDT_ISR                 ; Watchdog ISR

            .sect	".int02"				; FIGURE THIS OUT
            .word	P1_ISR					; Port 1 ISR

            .sect   ".reset"                ; PUC Vector
            .word   RESET                   ; RESET ISR
            .end
