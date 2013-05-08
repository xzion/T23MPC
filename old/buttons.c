///*
// * buttons.c
// *
// * contains functions to configure and run buttons/LEDs
// *
// * 	Button matrix:
// * 			GND1	GND2	GND3	GND4
// *	SWT4 	13		14		15		16
// * 	SWT3	9		10		11		12
// *	SWT2 	5		6		7		8
// * 	SWT1	1		2		3		4
// *
// */
//
///* Library headers */
//#include "inc/hw_ints.h"
//#include "inc/hw_types.h"
//#include "inc/hw_memmap.h"
//#include "driverlib/debug.h"
//#include "driverlib/gpio.h"
//#include "driverlib/interrupt.h"
//#include "driverlib/pin_map.h"
//#include "driverlib/rom.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/timer.h"
//
///* My file headers */
//#include "buttons.h"
//#include "sdc.h"
//#include "samples.h"
//#include "uart.h"
//
///* Definitions */
//	/* PORT A */
//#define BUTTON_SWT_GND4		GPIO_PIN_6
//#define BUTTON_LED_GND4		GPIO_PIN_7
//	/* PORT B */
//#define BUTTON_SWT_GND3		GPIO_PIN_0
//#define BUTTON_LED_GND3		GPIO_PIN_1
//#define BUTTON_SWT1			GPIO_PIN_2
//#define BUTTON_RED_2		GPIO_PIN_3
//#define BUTTON_TEMPO_LED	GPIO_PIN_6
//	/* PORT C */
//#define BUTTON_SWT3			GPIO_PIN_4
//#define BUTTON_GRN_3		GPIO_PIN_5
//#define BUTTON_RED_3		GPIO_PIN_6
//#define BUTTON_SWT4			GPIO_PIN_7
//	/* PORT D */
//#define BUTTON_SWT_GND1		GPIO_PIN_1
//#define BUTTON_LED_GND1		GPIO_PIN_2
//#define BUTTON_SWT_GND2		GPIO_PIN_3
//#define BUTTON_GRN_4		GPIO_PIN_6
//#define BUTTON_RED_4		GPIO_PIN_7
//	/* PORT E */
//#define BUTTON_GRN_1		GPIO_PIN_0
//#define BUTTON_LED_GND2		GPIO_PIN_1
//	/* PORT F */
//#define BUTTON_RED_1		GPIO_PIN_0
//#define BUTTON_LOOP			GPIO_PIN_1
//#define BUTTON_SWT2			GPIO_PIN_2
//#define BUTTON_GRN_2		GPIO_PIN_3
//#define BUTTON_FUNCTION		GPIO_PIN_4
//
///* Variables */
//extern volatile unsigned long g_ulTimeStamp;
//unsigned long g_ulFunctionDebounce = 0, g_ulLoopDebounce = 0, g_ulLastLoop = 0;
//unsigned short g_usLooping = 0;
//unsigned char g_ucButtonLoop = 0;
//
//
///*
// * Initialises the button and LED GPIOs
// */
//void init_buttons(void) {
//	/* Enable ports */
//	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
//	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
//	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
//	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
//	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
//	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
//	/* Unlock GPIOCR registers */
//	HWREG(GPIO_PORTD_BASE + 0x520) = 0x4C4F434B;
//	HWREG(GPIO_PORTF_BASE + 0x520) = 0x4C4F434B;
//	/* Allow access to PF0 and PD7 */
//	HWREG(GPIO_PORTD_BASE + 0x524) = 0x000000FF;
//	HWREG(GPIO_PORTF_BASE + 0x524) = 0x000000FF;
//	/* Lock GPIOCR registers */
//	HWREG(GPIO_PORTD_BASE + 0x520) = 0x00000000;
//	HWREG(GPIO_PORTF_BASE + 0x520) = 0x00000000;
//	/* Configure inputs - Function and Loop buttons and switch grounds */
//	ROM_GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, BUTTON_SWT_GND4);
//	ROM_GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, BUTTON_SWT_GND3);
//	ROM_GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, BUTTON_SWT_GND1 | BUTTON_SWT_GND2);
//	ROM_GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, BUTTON_LOOP | BUTTON_FUNCTION);
//	/* Configure outputs - LEDs and switches */
//	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, BUTTON_LED_GND4);
//	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, BUTTON_LED_GND3 | BUTTON_SWT1 | BUTTON_RED_2 | BUTTON_TEMPO_LED);
//	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, BUTTON_SWT3 | BUTTON_GRN_3 | BUTTON_RED_3 | BUTTON_SWT4);
//	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTD_BASE, BUTTON_LED_GND1 | BUTTON_GRN_4 | BUTTON_RED_4);
//	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTE_BASE, BUTTON_GRN_1 | BUTTON_LED_GND2);
//	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, BUTTON_RED_1 | BUTTON_SWT2 | BUTTON_GRN_2);
//	/* Set all switches and LEDs to ground, LED GNDs to Vcc */
//	ROM_GPIOPinWrite(GPIO_PORTA_BASE, BUTTON_LED_GND4, BUTTON_LED_GND4);
//	ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_LED_GND3 | BUTTON_SWT1 | BUTTON_RED_2 | BUTTON_TEMPO_LED, BUTTON_LED_GND3);
//	ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_SWT3 | BUTTON_GRN_3 | BUTTON_RED_3 | BUTTON_SWT4, 0);
//	ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_LED_GND1 | BUTTON_GRN_4 | BUTTON_RED_4, BUTTON_LED_GND1);
//	ROM_GPIOPinWrite(GPIO_PORTE_BASE, BUTTON_GRN_1 | BUTTON_LED_GND2, BUTTON_LED_GND2);
//	ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_RED_1 | BUTTON_SWT2 | BUTTON_GRN_2, 0);
//}
//
///*
// * Scans through the buttons and writes to an array with each button corresponding to a bit position
// * 0 = off, 1 = on, 2 = loop, 3 = function
// *
// * Takes a pointer to a buttons array of size 16 shorts, no memory is allocated here
// * Takes a pointer to the debouncing array (time since last press) of size 16 longs, no memory allocated here
// * Debouncing is set to 100 ms
// * Function should be called often to allow for debouncing
// */
//void scan_buttons(unsigned char* pucButtons, unsigned long* pulDebounce) {
//	unsigned char a = g_ucButtonLoop++ % 4;
//	/* For each row, set to Vcc and check each column */
//	if(a == 0) {
//		ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_SWT1, BUTTON_SWT1);
//		pucButtons[0] = ROM_GPIOPinRead(GPIO_PORTD_BASE, BUTTON_SWT_GND1);
//		pucButtons[1] = ROM_GPIOPinRead(GPIO_PORTD_BASE, BUTTON_SWT_GND2);
//		pucButtons[2] = ROM_GPIOPinRead(GPIO_PORTB_BASE, BUTTON_SWT_GND3);
//		pucButtons[3] = ROM_GPIOPinRead(GPIO_PORTA_BASE, BUTTON_SWT_GND4);
//		ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_SWT1, 0);
//	} else if(a == 1) {
//		ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_SWT2, BUTTON_SWT2);
//		pucButtons[4] = ROM_GPIOPinRead(GPIO_PORTD_BASE, BUTTON_SWT_GND1);
//		pucButtons[5] = ROM_GPIOPinRead(GPIO_PORTD_BASE, BUTTON_SWT_GND2);
//		pucButtons[6] = ROM_GPIOPinRead(GPIO_PORTB_BASE, BUTTON_SWT_GND3);
//		pucButtons[7] = ROM_GPIOPinRead(GPIO_PORTA_BASE, BUTTON_SWT_GND4);
//		ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_SWT2, 0);
//	} else if(a == 2) {
//		ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_SWT3, BUTTON_SWT3);
//		pucButtons[8] = ROM_GPIOPinRead(GPIO_PORTD_BASE, BUTTON_SWT_GND1);
//		pucButtons[9] = ROM_GPIOPinRead(GPIO_PORTD_BASE, BUTTON_SWT_GND2);
//		pucButtons[10] = ROM_GPIOPinRead(GPIO_PORTB_BASE, BUTTON_SWT_GND3);
//		pucButtons[11] = ROM_GPIOPinRead(GPIO_PORTA_BASE, BUTTON_SWT_GND4);
//		ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_SWT3, 0);
//	} else if(a == 3) {
//		ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_SWT4, BUTTON_SWT4);
//		pucButtons[12] = ROM_GPIOPinRead(GPIO_PORTD_BASE, BUTTON_SWT_GND1);
//		pucButtons[13] = ROM_GPIOPinRead(GPIO_PORTD_BASE, BUTTON_SWT_GND2);
//		pucButtons[14] = ROM_GPIOPinRead(GPIO_PORTB_BASE, BUTTON_SWT_GND3);
//		pucButtons[15] = ROM_GPIOPinRead(GPIO_PORTA_BASE, BUTTON_SWT_GND4);
//		ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_SWT4, 0);
//	}
//
//	/* debounce each button and set to 2 or 3 if loop or function respectively */
//	unsigned char i = 0, ucValue = 1;
//	unsigned long ulTime = g_ulTimeStamp;
//	if(ROM_GPIOPinRead(GPIO_PORTF_BASE, BUTTON_LOOP)) {
//		if(g_ulLoopDebounce == 0) {
//			g_ulLoopDebounce = ulTime;
//		}
//		if(ulTime - g_ulLoopDebounce > 5) {
//			ucValue = 2;
//		}
//	} else {
//		g_ulLoopDebounce = 0;
//	}
//	if(ROM_GPIOPinRead(GPIO_PORTF_BASE, BUTTON_FUNCTION)) {
//		if(g_ulFunctionDebounce == 0) {
//			g_ulFunctionDebounce = ulTime;
//		}
//		if(ulTime - g_ulFunctionDebounce > 5) {
//			ucValue = 3;
//		}
//	} else {
//		g_ulFunctionDebounce = 0;
//	}
//	for(i = a*4; i < (a+1)*4; ++i) {
//		/* If button has been pressed, ignore if it hasn't been pressed down for 40ms */
//		if(pucButtons[i]) {
//			pucButtons[i] = 0;
//			if(pulDebounce[i] == 0) {
//				pulDebounce[i] = ulTime;
//			} else if(ulTime - pulDebounce[i] >= 4) {
//				pucButtons[i] = ucValue;
//			}
//		} else {
//			pulDebounce[i] = 0;
//		}
//	}
//}
//
///*
// * Takes a button array (from scan_buttons) and processes the button inputs
// *
// * returns 1 if the user has requested a configuration update, 2 if loading
// *
// * 0 - stops playing HOLD samples
// * 1 - play sample
// * 2 - loop sample
// * 3 - function (if function assigned)
// */
//char process_buttons(unsigned char* pucButtons, unsigned char* pucLeds) {
//	unsigned char i;
//	for(i = 0; i < 16; ++i) {
//		pucLeds[i] = 0;
//		switch(pucButtons[i]) {
//		case 0:
//			/* Stop playing sample if hold and not looping */
//			if(!(g_usLooping & (1 << i)) && (g_sConfig.usHold & (1 << i))) {
//				stop_playing(i);
//			}
//			break;
//		case 1:
//			/* Play sample */
//			if(g_sConfig.ucSample1Index != i && g_sConfig.ucSample2Index != i) {
//				start_playing(i);
//			}
//			break;
//		case 2:
//			/* Toggle looping of sample */
//			if(g_ulTimeStamp - g_ulLastLoop < 100) {
//				break;
//			}
//			g_ulLastLoop = g_ulTimeStamp;
//			g_usLooping ^= 1 << i;
//			if(g_usLooping & (1 << i)) {
//				start_playing(i);
//			}
//			break;
//		case 3:
//			/* Function */
//			switch(i) {
//			case 1:
//				/* LFO */
//				pucLeds[i] = 2;
//				break;
//			case 2:
//				/* Stop all operation in preparation for configuration update */
//				pucLeds[i] = 2;
//				return 1;
//			case 3:
//				/* PC is loading configuration, wait until it's done */
//				pucLeds[i] = 2;
//				return 2;
//			default: break;
//			}
//			break;
//		default: break;
//		}
//		if(pucButtons[i] != 3 && (g_sConfig.ucSample1Index == i || g_sConfig.ucSample2Index == i)) {
//			pucLeds[i] = 1;
//		}
//	}
//	return 0;
//}
//
///*
// * Enables the LED corresponding to the current loop iteration
// *
// * Takes an array of LED states
// * 0 = off, 1 = LED1, 2 = LED2
// * Takes a loop iteration variable, determining which LED in the list should be activated
// */
//void set_leds(unsigned char* pucLeds, unsigned char ucLoop) {
//	unsigned char i = 0, ucLedsOn = 0, ucLedCount = 0;
//	/* Disable all previous leds */
//	ROM_GPIOPinWrite(GPIO_PORTA_BASE, BUTTON_LED_GND4, BUTTON_LED_GND4);
//	ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_LED_GND3 | BUTTON_RED_2, BUTTON_LED_GND3);
//	ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_GRN_3 | BUTTON_RED_3, 0);
//	ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_LED_GND1 | BUTTON_GRN_4 | BUTTON_RED_4, BUTTON_LED_GND1);
//	ROM_GPIOPinWrite(GPIO_PORTE_BASE, BUTTON_GRN_1 | BUTTON_LED_GND2, BUTTON_LED_GND2);
//	ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_RED_1 | BUTTON_GRN_2, 0);
//	/* Determine how many pucLeds are on, these are what we will by cycling through */
//	for(i = 0; i < 16; ++i) {
//		if(pucLeds[i]) ++ucLedsOn;
//	}
//	/* Quit if there are no leds to turn on */
//	if(ucLedsOn == 0) return;
//	/* Find out which LED the function is up to */
//	for(i = 0; ucLedCount != (ucLoop % ucLedsOn) + 1; ++i) {
//		if(pucLeds[i]) ++ucLedCount;
//	}
//	/* Enable the LED */
//	switch(--i) {
//	case 0: // LED1
//		ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_LED_GND1, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTE_BASE, BUTTON_GRN_1, BUTTON_GRN_1);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_RED_1, BUTTON_RED_1);
////		}
//		break;
//	case 1: // LED2
//		ROM_GPIOPinWrite(GPIO_PORTE_BASE, BUTTON_LED_GND2, 0);
//		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTE_BASE, BUTTON_GRN_1, BUTTON_GRN_1);
//		} else {
//			ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_RED_1, BUTTON_RED_1);
//		}
//		break;
//	case 2: // LED3
//		ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_LED_GND3, 0);
//		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTE_BASE, BUTTON_GRN_1, BUTTON_GRN_1);
//		} else {
//			ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_RED_1, BUTTON_RED_1);
//		}
//		break;
//	case 3: // LED4
//		ROM_GPIOPinWrite(GPIO_PORTA_BASE, BUTTON_LED_GND4, 0);
//		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTE_BASE, BUTTON_GRN_1, BUTTON_GRN_1);
//		} else {
//			ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_RED_1, BUTTON_RED_1);
//		}
//		break;
//	case 4: // LED5
//		ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_LED_GND1, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_GRN_2, BUTTON_GRN_2);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_RED_2, BUTTON_RED_2);
////		}
//		break;
//	case 5: // LED6
//		ROM_GPIOPinWrite(GPIO_PORTE_BASE, BUTTON_LED_GND2, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_GRN_2, BUTTON_GRN_2);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_RED_2, BUTTON_RED_2);
////		}
//		break;
//	case 6: // LED7
//		ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_LED_GND3, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_GRN_2, BUTTON_GRN_2);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_RED_2, BUTTON_RED_2);
////		}
//		break;
//	case 7: // LED8
//		ROM_GPIOPinWrite(GPIO_PORTA_BASE, BUTTON_LED_GND4, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTF_BASE, BUTTON_GRN_2, BUTTON_GRN_2);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_RED_2, BUTTON_RED_2);
////		}
//		break;
//	case 8: // LED9
//		ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_LED_GND1, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_GRN_3, BUTTON_GRN_3);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_RED_3, BUTTON_RED_3);
////		}
//		break;
//	case 9: // LED10
//		ROM_GPIOPinWrite(GPIO_PORTE_BASE, BUTTON_LED_GND2, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_GRN_3, BUTTON_GRN_3);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_RED_3, BUTTON_RED_3);
////		}
//		break;
//	case 10: // LED11
//		ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_LED_GND3, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_GRN_3, BUTTON_GRN_3);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_RED_3, BUTTON_RED_3);
////		}
//		break;
//	case 11: // LED12
//		ROM_GPIOPinWrite(GPIO_PORTA_BASE, BUTTON_LED_GND4, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_GRN_3, BUTTON_GRN_3);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTC_BASE, BUTTON_RED_3, BUTTON_RED_3);
////		}
//		break;
//	case 12: // LED13
//		ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_LED_GND1, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_GRN_4, BUTTON_GRN_4);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_RED_4, BUTTON_RED_4);
////		}
//		break;
//	case 13: // LED14
//		ROM_GPIOPinWrite(GPIO_PORTE_BASE, BUTTON_LED_GND2, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_GRN_4, BUTTON_GRN_4);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_RED_4, BUTTON_RED_4);
////		}
//		break;
//	case 14: // LED15
//		ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_LED_GND3, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_GRN_4, BUTTON_GRN_4);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_RED_4, BUTTON_RED_4);
////		}
//		break;
//	case 15: // LED16
//		ROM_GPIOPinWrite(GPIO_PORTA_BASE, BUTTON_LED_GND4, 0);
////		if(pucLeds[i] == 1) {
//			ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_GRN_4, BUTTON_GRN_4);
////		} else {
////			ROM_GPIOPinWrite(GPIO_PORTD_BASE, BUTTON_RED_4, BUTTON_RED_4);
////		}
//		break;
//	default:
//		break;
//	}
//}
