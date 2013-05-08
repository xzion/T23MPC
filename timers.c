/*
 * inits.c
 *
 *  Created on: 07/05/2013
 *  Author: 	Coen McClelland
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
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include <stdint.h>

// Local Includes
#include "timers.h"
#include "uart.h"
#include "sdcard.h"
#include "dac.h"

// Definitions
#define BUFFERUPDATE_FREQ			100
#define PKT_SIZE					44100/BUFFERUPDATE_FREQ
#define TEMPO_FREQUENCY				(tempo*128)/60

// Variables
extern volatile unsigned long g_ulTimeStamp;
volatile uint8_t ulTempoTimestamp = 0;

static uint16_t oBuff[PKT_SIZE*3];
uint16_t * startPtr = &oBuff[0];
uint16_t * readPtr = &oBuff[0];
uint16_t * writePtr = &oBuff[0];

extern uint32_t whereLast[16];
extern uint16_t pressed;
extern uint16_t playing;
extern uint16_t latchHold;
extern uint16_t looping;
extern uint8_t FX1mode;
extern uint8_t FX2mode;
extern uint8_t tempo;

// Debugging variables
uint8_t testTimer0 = 1;
uint8_t testTimer1 = 1;
uint8_t testTimer2 = 1;




void init_timers(void) {
	unsigned long ulDACPeriod, ulPktPeriod;

	// Initialise the output buffer
	int i;
	for (i = 0; i < PKT_SIZE*3; i++)
	{
		oBuff[i] = 0;
	}
	// Initialise pointer positions
	for (i = 0; i < 16; i++)
	{
		whereLast[i] = 0;
	}

	// Configure the DAC output timer - Timer 0
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	ulDACPeriod = (ROM_SysCtlClockGet() / 44100); // 44.1 kHz for DAC samples
	ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ulDACPeriod - 1);
	// Set up interrupt
	ROM_IntEnable(INT_TIMER0A);
	ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	// Enable Timer
	ROM_TimerEnable(TIMER0_BASE, TIMER_A);

	// Configure the SD Card buffer timer - Timer 1
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
	ulPktPeriod = (ROM_SysCtlClockGet() / BUFFERUPDATE_FREQ); // 100 Hz for DAC samples
	ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, ulPktPeriod);
	// Set up interrupt
	ROM_IntEnable(INT_TIMER1A);
	ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	// Enable Timer
	ROM_TimerEnable(TIMER1_BASE, TIMER_A);

	// Configure the Tempo timer - Timer 2
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	ROM_TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
	ulPktPeriod = (ROM_SysCtlClockGet() / TEMPO_FREQUENCY);
	ROM_TimerLoadSet(TIMER2_BASE, TIMER_A, ulPktPeriod);
	// Set up interrupt
	ROM_IntEnable(INT_TIMER2A);
	ROM_TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	// Enable Timer
	ROM_TimerEnable(TIMER2_BASE, TIMER_A);

}

void timer0_int_handler(void) {
	// Clear the interrupt flag
	ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	// Pass the next value to the DAC
	dac_write(*readPtr);
	readPtr++;

	if (readPtr == startPtr + PKT_SIZE*3)
	{
		readPtr = startPtr;
		//UARTprintf("readPtr reset\n");
	}

	// Debugging
	if (g_ulTimeStamp % 100 == 0)
	{
		if (testTimer0)
		{
			UARTprintf("Timer 0 is firing\n");
			testTimer0 = 0;
		}
	}
	else
	{
		testTimer0 = 1;
	}
}

void timer1_int_handler(void) {
	// Clear the interrupt flag
	ROM_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	int i;
	uint16_t pkt1[PKT_SIZE];
	uint16_t pkt2[PKT_SIZE];
	uint16_t finalPkt[PKT_SIZE];

	// Initialise output packets
	for (i = 0; i < PKT_SIZE; i++)
	{
		pkt1[i] = 0;
		pkt2[i] = 0;
	}

	// Retrieve the current 2 audio streams
	for (i = 0; i < 16; i++)
	{
		uint8_t bitMask = 0 | (1 << i);
		uint8_t fileEnded = 0;
		FIL file;
		sdcard_openFile(&file, i);

		if (!!(pressed & bitMask))
		{
			// Button is pressed
			// NEED ORDER CHECK ON ALL OF THESE
//			FIL file;
//			sdcard_openFile(&file, i);
			sdcard_readPacket(&file, i, &finalPkt[0], PKT_SIZE);
//			sdcard_closeFile(&file);
			playing = playing | bitMask;
			UARTprintf("READING!, playing = 0x%X\n", playing);
		}
		else
		{
			//UARTprintf("unpressed, playing = 0x%x, bitmask = 0x%x, res = 0x%x\n", playing, bitMask, (playing & ~bitMask));
			// Button is not pressed, but may still be playing
			if (!!(playing & bitMask) && !!(latchHold & bitMask))
			{
				// File is latched, need to finish playback
//				FIL file;
//				sdcard_openFile(&file, i);
				fileEnded = sdcard_readPacket(&file, i, &finalPkt[0], PKT_SIZE);
//				sdcard_closeFile(&file);
				UARTprintf("FILLING!, WL = %d, playing = 0x%X, fe = %d\n", whereLast[i], playing, fileEnded);
				if (fileEnded)
				{
					// File has completed playback, stop it playing next time!
					// Should make sure this bitbashing checks out
					playing = playing & ~bitMask;
					whereLast[i] = 0;

					// Debugging
					UARTprintf("File %d ended!\n", i);

				}
				else
				{
					playing = playing | bitMask;
				}
			}
			else
			{
				// It's not playing
				playing = playing & ~bitMask;
			}
		}
		sdcard_closeFile(&file);
	}

	// DEBUGGING - unpress first button
	pressed = 0x00;


	// At this point, we have 2 arrays of the most up to date audio streams
	// Need to apply convolution, then FX
	// NOT DONE YET, OBVIOUSLY. ONLY HAVE 1 PKT AT THE MOMENT

	for (i = 0; i < PKT_SIZE; i++)
	{
		// Shift into the output buffer
		*writePtr = finalPkt[i];
		writePtr++;
		if (writePtr == startPtr + PKT_SIZE*3)
		{
			writePtr = startPtr;
			//UARTprintf("readPtr reset\n");
		}
	}


	// Debugging
	if (g_ulTimeStamp % 100 == 0)
	{
		if (testTimer1)
		{
			UARTprintf("Timer 1 is firing\n");
			testTimer1 = 0;
		}
	}
	else
	{
		testTimer1 = 1;
	}
}

void timer2_int_handler(void) {
	// Clear the interrupt flag
	ROM_TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

	ulTempoTimestamp++;
	ulTempoTimestamp = ulTempoTimestamp % 128;

	if (ulTempoTimestamp == 0)
	{
		// TOGGLE THE TEMPO LED ON

	}
	else if (ulTempoTimestamp == 16)
	{
		// TOGGLE THE TEMPO LED OFF

	}

	// Debugging
	if (g_ulTimeStamp % 100 == 0)
	{
		if (testTimer2)
		{
			UARTprintf("Timer 2 is firing\n");
			testTimer2 = 0;
		}
	}
	else
	{
		testTimer2 = 1;
	}
}



