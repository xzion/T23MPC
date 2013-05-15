/*
 * btn.c
 *
 *  Created on: 12/05/2013
 *      Author: Coen
 */

// Library Includes
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include <stdint.h>

// Local Includes
#include "btn.h"
#include "uart.h"

// Definitions
#define BTN_ROW1_RD			GPIO_PORTE_BASE, GPIO_PIN_0 // This is not as listed on the DOC, PD0 doesn't work though.
#define BTN_ROW2_RD			GPIO_PORTD_BASE, GPIO_PIN_1
#define BTN_ROW3_RD			GPIO_PORTD_BASE, GPIO_PIN_2
#define BTN_ROW4_RD			GPIO_PORTD_BASE, GPIO_PIN_3

#define BTN_COL1_IN			GPIO_PORTF_BASE, GPIO_PIN_0
#define BTN_COL2_IN			GPIO_PORTF_BASE, GPIO_PIN_1
#define BTN_COL3_IN			GPIO_PORTF_BASE, GPIO_PIN_2
#define BTN_COL4_IN			GPIO_PORTF_BASE, GPIO_PIN_3

#define LED_ROW1_RED		GPIO_PORTA_BASE, GPIO_PIN_6
#define LED_ROW2_RED		GPIO_PORTB_BASE, GPIO_PIN_0
#define LED_ROW3_RED		GPIO_PORTB_BASE, GPIO_PIN_2
#define LED_ROW4_RED		GPIO_PORTC_BASE, GPIO_PIN_4

#define LED_ROW1_BLUE		GPIO_PORTA_BASE, GPIO_PIN_7
#define LED_ROW2_BLUE		GPIO_PORTB_BASE, GPIO_PIN_1
#define LED_ROW3_BLUE		GPIO_PORTB_BASE, GPIO_PIN_3
#define LED_ROW4_BLUE		GPIO_PORTC_BASE, GPIO_PIN_5

#define LED_COL1_GND		GPIO_PORTC_BASE, GPIO_PIN_6
#define LED_COL2_GND		GPIO_PORTC_BASE, GPIO_PIN_7
#define LED_COL3_GND		GPIO_PORTD_BASE, GPIO_PIN_6
#define LED_COL4_GND		GPIO_PORTD_BASE, GPIO_PIN_7

#define BTN_LOOP			GPIO_PORTE_BASE, GPIO_PIN_4
#define BTN_FN				GPIO_PORTB_BASE, GPIO_PIN_6

// Variables
extern uint16_t pressed;
extern uint16_t playing;
extern uint16_t looping;
extern uint16_t loopMod;
extern uint32_t lastPressTs[16];
extern volatile unsigned long g_ulTimeStamp;
uint8_t currentRow;
uint8_t ledRow;
uint8_t bounceStatus[4][4];
uint8_t fnOn;





// Initialise GPIO for the button pad and extra pad
void btn_init(void) {
	// Configure FN, LOOP and button reads and inputs
	ROM_GPIOPinTypeGPIOInput(BTN_ROW1_RD);
	ROM_GPIOPinTypeGPIOInput(BTN_ROW2_RD);
	ROM_GPIOPinTypeGPIOInput(BTN_ROW3_RD);
	ROM_GPIOPinTypeGPIOInput(BTN_ROW4_RD);
	ROM_GPIOPinTypeGPIOInput(BTN_LOOP);
	ROM_GPIOPinTypeGPIOInput(BTN_FN);

	// Configure all of the other pins as output
	ROM_GPIOPinTypeGPIOOutput(BTN_COL1_IN);
	ROM_GPIOPinTypeGPIOOutput(BTN_COL2_IN);
	ROM_GPIOPinTypeGPIOOutput(BTN_COL3_IN);
	ROM_GPIOPinTypeGPIOOutput(BTN_COL4_IN);

	ROM_GPIOPinTypeGPIOOutput(LED_ROW1_RED);
	ROM_GPIOPinTypeGPIOOutput(LED_ROW2_RED);
	ROM_GPIOPinTypeGPIOOutput(LED_ROW3_RED);
	ROM_GPIOPinTypeGPIOOutput(LED_ROW4_RED);

	ROM_GPIOPinTypeGPIOOutput(LED_ROW1_BLUE);
	ROM_GPIOPinTypeGPIOOutput(LED_ROW2_BLUE);
	ROM_GPIOPinTypeGPIOOutput(LED_ROW3_BLUE);
	ROM_GPIOPinTypeGPIOOutput(LED_ROW4_BLUE);

	ROM_GPIOPinTypeGPIOOutput(LED_COL1_GND);
	ROM_GPIOPinTypeGPIOOutput(LED_COL2_GND);
	ROM_GPIOPinTypeGPIOOutput(LED_COL3_GND);
	ROM_GPIOPinTypeGPIOOutput(LED_COL4_GND);

	// Set Button inputs to pulldown resistors
	ROM_GPIOPadConfigSet(BTN_ROW1_RD, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPD);
	ROM_GPIOPadConfigSet(BTN_ROW2_RD, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPD);
	ROM_GPIOPadConfigSet(BTN_ROW3_RD, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPD);
	ROM_GPIOPadConfigSet(BTN_ROW4_RD, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPD);

	// Initialise output pins to 0, LED grounds to 1
	ROM_GPIOPinWrite(BTN_COL1_IN, 0x00);
	ROM_GPIOPinWrite(BTN_COL2_IN, 0x00);
	ROM_GPIOPinWrite(BTN_COL3_IN, 0x00);
	ROM_GPIOPinWrite(BTN_COL4_IN, 0x00);

	ROM_GPIOPinWrite(LED_ROW1_RED, 0x00);
	ROM_GPIOPinWrite(LED_ROW2_RED, 0x00);
	ROM_GPIOPinWrite(LED_ROW3_RED, 0x00);
	ROM_GPIOPinWrite(LED_ROW4_RED, 0x00);

	ROM_GPIOPinWrite(LED_ROW1_BLUE, 0x00);
	ROM_GPIOPinWrite(LED_ROW2_BLUE, 0x00);
	ROM_GPIOPinWrite(LED_ROW3_BLUE, 0x00);
	ROM_GPIOPinWrite(LED_ROW4_BLUE, 0x00);

	ROM_GPIOPinWrite(LED_COL1_GND, 0xFF);
	ROM_GPIOPinWrite(LED_COL2_GND, 0xFF);
	ROM_GPIOPinWrite(LED_COL3_GND, 0xFF);
	ROM_GPIOPinWrite(LED_COL4_GND, 0xFF);

	// Initialise variables
	currentRow = 0;
	fnOn = 0;
	ledRow = 0;


	// Set first button row up for reading
	ROM_GPIOPinWrite(BTN_COL1_IN, 0xFF);
	// Ground first LED Row for display
	ROM_GPIOPinWrite(LED_COL1_GND, 0x00);

}

void btn_pollRow(void) {
	int i;
	// Poll the buttons
	uint8_t loopOn = !!(ROM_GPIOPinRead(BTN_LOOP));

	uint8_t btnRd[4];
	btnRd[0] = !!(ROM_GPIOPinRead(BTN_ROW1_RD));
	btnRd[1] = !!(ROM_GPIOPinRead(BTN_ROW2_RD));
	btnRd[2] = !!(ROM_GPIOPinRead(BTN_ROW3_RD));
	btnRd[3] = !!(ROM_GPIOPinRead(BTN_ROW4_RD));

	// THIS WONT WORK, WILL FLICKER AND BOUNCE
//	if (!!(ROM_GPIOPinRead(BTN_FN)))
//	{
//		fnOn = !fnOn;
//	}
	fnOn = !!(ROM_GPIOPinRead(BTN_FN));

	// Update our arrays
	if (currentRow == 0)
	{
		for (i = 0; i < 4; i++)
		{
			// Shift the debounced value into pressed
			uint8_t pinStatus = debounce(i, btnRd[i]);
			pressed = (pressed & ~(1 << i)) | (pinStatus << i);
			if (loopOn && pinStatus && !!(loopMod & (1 << i)))
			{
				if (!!(looping & (1 << i)))
				{
					looping &= ~(1 << i);
					UARTprintf("LOOPING OFF row: %d, col: %d\n", currentRow, i);
				}
				else
				{
					looping |= (1 << i);
					UARTprintf("LOOPING ON row: %d, col: %d\n", currentRow, i);
					loopMod &= ~(1 << i);
				}
			}
		}

		currentRow++;
		ROM_GPIOPinWrite(BTN_COL1_IN, 0x00);
		ROM_GPIOPinWrite(BTN_COL2_IN, 0xFF);
	}
	else if (currentRow == 1)
	{
		for (i = 0; i < 4; i++)
		{
			// Shift the debounced value into pressed
			uint8_t pinStatus = debounce(i, btnRd[i]);
			pressed = (pressed & ~(1 << i+4)) | (pinStatus << i+4);
			if (loopOn && pinStatus && !!(loopMod & (1 << i+4)))
			{
				if (!!(looping & (1 << i+4)))
				{
					looping &= ~(1 << i+4);
					UARTprintf("LOOPING OFF row: %d, col: %d\n", currentRow, i);
				}
				else
				{
					looping |= (1 << i+4);
					UARTprintf("LOOPING ON row: %d, col: %d\n", currentRow, i);
					loopMod &= ~(1 << i+4);
				}
			}
		}

		currentRow++;
		ROM_GPIOPinWrite(BTN_COL2_IN, 0x00);
		ROM_GPIOPinWrite(BTN_COL3_IN, 0xFF);
	}
	else if (currentRow == 2)
	{
		for (i = 0; i < 4; i++)
		{
			// Shift the debounced value into pressed
			uint8_t pinStatus = debounce(i, btnRd[i]);
			pressed = (pressed & ~(1 << i+8)) | (pinStatus << i+8);
			if (loopOn && pinStatus && !!(loopMod & (1 << i+8)))
			{
				if (!!(looping & (1 << i+8)))
				{
					looping &= ~(1 << i+8);
					UARTprintf("LOOPING OFF row: %d, col: %d\n", currentRow, i);
				}
				else
				{
					looping |= (1 << i+8);
					UARTprintf("LOOPING ON row: %d, col: %d\n", currentRow, i);
					loopMod &= ~(1 << i+8);
				}
			}
		}

		currentRow++;
		ROM_GPIOPinWrite(BTN_COL3_IN, 0x00);
		ROM_GPIOPinWrite(BTN_COL4_IN, 0xFF);
		}
	else
	{
		for (i = 0; i < 4; i++)
		{
			// Shift the debounced value into pressed
			uint8_t pinStatus = debounce(i, btnRd[i]);
			pressed = (pressed & ~(1 << i+12)) | (pinStatus << i+12);
			if (loopOn && pinStatus && !!(loopMod & (1 << i+12)))
			{
				if (!!(looping & (1 << i+12)))
				{
					looping &= ~(1 << i+12);
					UARTprintf("LOOPING OFF row: %d, col: %d\n", currentRow, i);
				}
				else
				{
					looping |= (1 << i+12);
					UARTprintf("LOOPING ON row: %d, col: %d\n", currentRow, i);
					loopMod &= ~(1 << i+12);
				}
			}
		}

		currentRow = 0;
		ROM_GPIOPinWrite(BTN_COL4_IN, 0x00);
		ROM_GPIOPinWrite(BTN_COL1_IN, 0xFF);
	}
}

uint8_t debounce(uint8_t btnNum, uint8_t rawInput) {
	// RAW
	if(rawInput)
	{
		UARTprintf("SOMETHING IS HIGH %d %d\n", currentRow, btnNum);
		lastPressTs[currentRow*4+btnNum] = g_ulTimeStamp;
		return 1;
	}
	return 0;


//	// Bitshifting left, requires one bit high
//	bounceStatus[currentRow][btnNum] = (bounceStatus[currentRow][btnNum] << 1) | rawInput;
//	if(rawInput)
//	{
//		UARTprintf("SOMETHING IS HIGH %d %d\n", currentRow, btnNum);
//	}
//
//	if (bounceStatus[currentRow][btnNum] == 0x00)
//	{
//		return 0;
//	}
//	//UARTprintf("PRESSED row: %d, col: %d\n", currentRow, btnNum);
//	// Set last press status!
//	lastPressTs[currentRow*4+btnNum] = g_ulTimeStamp;
//	return 1;

//	// Bitshifting right, requires 2 high
//	bounceStatus[currentRow][btnNum] = (bounceStatus[currentRow][btnNum] >> 1) | (rawInput << 7);
//	if(rawInput)
//	{
//		UARTprintf("SOMETHING IS HIGH %d %d\n", currentRow, btnNum);
//	}
//
//	if (bounceStatus[currentRow][btnNum] > 0x80)
//	{
////		UARTprintf("PRESSED row: %d, col: %d\n", currentRow, btnNum);
//		// Set last press status!
//		lastPressTs[currentRow*4+btnNum] = g_ulTimeStamp;
//		return 1;
//	}
//	return 0;
}

void update_LEDs(void) {
	// Turn everything off
	ROM_GPIOPinWrite(LED_ROW1_RED, 0x00);
	ROM_GPIOPinWrite(LED_ROW2_RED, 0x00);
	ROM_GPIOPinWrite(LED_ROW3_RED, 0x00);
	ROM_GPIOPinWrite(LED_ROW4_RED, 0x00);

	ROM_GPIOPinWrite(LED_ROW1_BLUE, 0x00);
	ROM_GPIOPinWrite(LED_ROW2_BLUE, 0x00);
	ROM_GPIOPinWrite(LED_ROW3_BLUE, 0x00);
	ROM_GPIOPinWrite(LED_ROW4_BLUE, 0x00);

	ROM_GPIOPinWrite(LED_COL1_GND, 0xFF);
	ROM_GPIOPinWrite(LED_COL2_GND, 0xFF);
	ROM_GPIOPinWrite(LED_COL3_GND, 0xFF);
	ROM_GPIOPinWrite(LED_COL4_GND, 0xFF);

	if (ledRow == 0)
	{
		// Ground this line
		ROM_GPIOPinWrite(LED_COL1_GND, 0x00);
		if(fnOn)
		{
			// No super easy way to do this, gotta hardcode the shifts (or add a heap more defines)
			ROM_GPIOPinWrite(LED_ROW1_BLUE, ((!!(playing & 0x0001)) << 7));
			ROM_GPIOPinWrite(LED_ROW2_BLUE, ((!!(playing & 0x0002)) << 1));
			ROM_GPIOPinWrite(LED_ROW3_BLUE, ((!!(playing & 0x0004)) << 3));
			ROM_GPIOPinWrite(LED_ROW4_BLUE, ((!!(playing & 0x0008)) << 5));
		}
		else
		{
			ROM_GPIOPinWrite(LED_ROW1_RED, ((!!(playing & 0x0001)) << 6));
			ROM_GPIOPinWrite(LED_ROW2_RED, ((!!(playing & 0x0002)) << 0));
			ROM_GPIOPinWrite(LED_ROW3_RED, ((!!(playing & 0x0004)) << 2));
			ROM_GPIOPinWrite(LED_ROW4_RED, ((!!(playing & 0x0008)) << 4));
		}
	}
	else if (ledRow == 1)
	{
		// Ground this line
		ROM_GPIOPinWrite(LED_COL2_GND, 0x00);
		if(fnOn)
		{
			// No super easy way to do this, gotta hardcode the shifts (or add a heap more defines)
			ROM_GPIOPinWrite(LED_ROW1_BLUE, ((!!(playing & 0x0010)) << 7));
			ROM_GPIOPinWrite(LED_ROW2_BLUE, ((!!(playing & 0x0020)) << 1));
			ROM_GPIOPinWrite(LED_ROW3_BLUE, ((!!(playing & 0x0040)) << 3));
			ROM_GPIOPinWrite(LED_ROW4_BLUE, ((!!(playing & 0x0080)) << 5));
		}
		else
		{
			ROM_GPIOPinWrite(LED_ROW1_RED, ((!!(playing & 0x0010)) << 6));
			ROM_GPIOPinWrite(LED_ROW2_RED, ((!!(playing & 0x0020)) << 0));
			ROM_GPIOPinWrite(LED_ROW3_RED, ((!!(playing & 0x0040)) << 2));
			ROM_GPIOPinWrite(LED_ROW4_RED, ((!!(playing & 0x0080)) << 4));
		}
	}
	else if (ledRow == 2)
	{
		// Ground this line
		ROM_GPIOPinWrite(LED_COL3_GND, 0x00);
		if(fnOn)
		{
			// No super easy way to do this, gotta hardcode the shifts (or add a heap more defines)
			ROM_GPIOPinWrite(LED_ROW1_BLUE, ((!!(playing & 0x0100)) << 7));
			ROM_GPIOPinWrite(LED_ROW2_BLUE, ((!!(playing & 0x0200)) << 1));
			ROM_GPIOPinWrite(LED_ROW3_BLUE, ((!!(playing & 0x0400)) << 3));
			ROM_GPIOPinWrite(LED_ROW4_BLUE, ((!!(playing & 0x0800)) << 5));
		}
		else
		{
			ROM_GPIOPinWrite(LED_ROW1_RED, ((!!(playing & 0x0100)) << 6));
			ROM_GPIOPinWrite(LED_ROW2_RED, ((!!(playing & 0x0200)) << 0));
			ROM_GPIOPinWrite(LED_ROW3_RED, ((!!(playing & 0x0400)) << 2));
			ROM_GPIOPinWrite(LED_ROW4_RED, ((!!(playing & 0x0800)) << 4));
		}
	}
	else
	{
		// Ground this line
		ROM_GPIOPinWrite(LED_COL4_GND, 0x00);
		if(fnOn)
		{
			// No super easy way to do this, gotta hardcode the shifts (or add a heap more defines)
			ROM_GPIOPinWrite(LED_ROW1_BLUE, ((!!(playing & 0x1000)) << 6));
			ROM_GPIOPinWrite(LED_ROW2_BLUE, ((!!(playing & 0x2000)) << 0));
			ROM_GPIOPinWrite(LED_ROW3_BLUE, ((!!(playing & 0x4000)) << 2));
			ROM_GPIOPinWrite(LED_ROW4_BLUE, ((!!(playing & 0x8000)) << 4));
		}
		else
		{
			ROM_GPIOPinWrite(LED_ROW1_RED, ((!!(playing & 0x1000)) << 6));
			ROM_GPIOPinWrite(LED_ROW2_RED, ((!!(playing & 0x2000)) << 0));
			ROM_GPIOPinWrite(LED_ROW3_RED, ((!!(playing & 0x4000)) << 2));
			ROM_GPIOPinWrite(LED_ROW4_RED, ((!!(playing & 0x8000)) << 4));
		}
	}
	ledRow = (ledRow + 1) % 4;
}


