// dice.h	03/15/2014
//*******************************************************************************
//
//                            MSP430F2274
//                  .-----------------------------.
//            SW1-->|P1.0^                    P2.0|<->LCD_DB0
//            SW2-->|P1.1^                    P2.1|<->LCD_DB1
//            SW3-->|P1.2^                    P2.2|<->LCD_DB2
//            SW4-->|P1.3^                    P2.3|<->LCD_DB3
//       ADXL_INT-->|P1.4                     P2.4|<->LCD_DB4
//        AUX INT-->|P1.5                     P2.5|<->LCD_DB5
//        SERVO_1<--|P1.6 (TA1)               P2.6|<->LCD_DB6
//        SERVO_2<--|P1.7 (TA2)               P2.7|<->LCD_DB7
//                  |                             |
//         LCD_A0<--|P3.0                     P4.0|-->LED_1 (Green)
//        i2c_SDA<->|P3.1 (UCB0SDA)     (TB1) P4.1|-->LED_2 (Orange) / SERVO_3
//        i2c_SCL<--|P3.2 (UCB0SCL)     (TB2) P4.2|-->LED_3 (Yellow) / SERVO_4
//         LCD_RW<--|P3.3                     P4.3|-->LED_4 (Red)
//   TX/LED_5 (G)<--|P3.4 (UCA0TXD)     (TB1) P4.4|-->LCD_BL
//             RX-->|P3.5 (UCA0RXD)     (TB2) P4.5|-->SPEAKER
//           RPOT-->|P3.6 (A6)          (A15) P4.6|-->LED 6 (R)
//           LPOT-->|P3.7 (A7)                P4.7|-->LCD_E
//                  '-----------------------------'
//
//******************************************************************************

#ifndef DICE_H_
#define DICE_H_

// defined constants -----------------------------------------------------------
#define myCLOCK			8000000			// 8 Mhz clock
#define	WDT_CTL			WDT_MDLY_32		// WD configuration (Timer, SMCLK, ~32 ms)
#define WDT_CPI			32000			// WDT Clocks Per Interrupt (@1 Mhz)
#define	WDT_1SEC_CNT	myCLOCK/WDT_CPI	// WDT counts/second (32 ms)

// function prototypes ---------------------------------------------------------
void WDT_delay(uint16 delay);			// WDT delay routine
void doTone(uint16 tone, uint16 time);	// output tone
int WDT_init(void);
int timerB_init(void);
void drawDie(uint8 die, int16 x, int16 y);

// external variables ------------------------------------------------------------
extern const uint16 spot_image[];
extern const uint16 byu4_image[];

#define BEEP		(1000*(myCLOCK/1000000))	// beep frequency
#define BEEP_CNT	(5*(myCLOCK/1000000))		// beep duration

// C chromatic scale -------------------------------------------------------------
#define TONE_C		myCLOCK/1308/12*10
#define TONE_Cs		myCLOCK/1386/12*10
#define TONE_D		myCLOCK/1468/12*10
#define TONE_Eb		myCLOCK/1556/12*10
#define TONE_E		myCLOCK/1648/12*10
#define TONE_F		myCLOCK/1746/12*10
#define TONE_Fs		myCLOCK/1850/12*10
#define TONE_G		myCLOCK/1950/12*10
#define TONE_Ab		myCLOCK/2077/12*10
#define TONE_A		myCLOCK/2200/12*10
#define TONE_Bb		myCLOCK/2331/12*10
#define TONE_B		myCLOCK/2469/12*10
#define TONE_C1		myCLOCK/2616/12*10
#define TONE_Cs1	myCLOCK/2772/12*10
#define TONE_D1		myCLOCK/2937/12*10
#define TONE_Eb1	myCLOCK/3111/12*10
#define TONE_E1		myCLOCK/3296/12*10
#define TONE_F1		myCLOCK/3492/12*10
#define TONE_Fs1	myCLOCK/3700/12*10
#define TONE_G1		myCLOCK/3920/12*10
#define TONE_Ab1	myCLOCK/4150/12*10
#define TONE_A1		myCLOCK/4400/12*10
#define TONE_Bb1	myCLOCK/4662/12*10
#define TONE_B1		myCLOCK/4939/12*10

#endif /* DICE_H_ */
