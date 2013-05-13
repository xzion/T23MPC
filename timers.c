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
#include "btn.h"

// Definitions
#define TEMPO_MULTIPLIER			640
#define TEMPO_FREQUENCY				(int)((tempo*TEMPO_MULTIPLIER)/60)
#define BUFFER_SIZE					PKT_SIZE*2
#define POLL_FREQUENCY				10

#define LED_TEMPO					GPIO_PORTF_BASE, GPIO_PIN_4

// Variables
extern volatile unsigned long g_ulTimeStamp;
uint16_t ulTempoTimestamp = 0;

static uint16_t oBuff[BUFFER_SIZE];
uint16_t * startPtr = &oBuff[0];
uint16_t * readPtr = &oBuff[0];
uint16_t * writePtr = &oBuff[0];
uint8_t readPoint = 0;
uint8_t pollCount = 0;

extern uint32_t whereLastPress[16];
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
uint16_t testSample = 0;




void timers_init(void) {
	unsigned long ulDACPeriod, ulPktPeriod, ulTempoPeriod, ulPollPeriod;

	// Initialise the output buffer
	int i;
	for (i = 0; i < BUFFER_SIZE; i++)
	{
		oBuff[i] = 0x8000;
	}
	// Initialise pointer positions
	for (i = 0; i < 16; i++)
	{
		whereLastPress[i] = 0;
	}

	// Configure the Tempo LED
	ROM_GPIOPinTypeGPIOOutput(LED_TEMPO);
	ROM_GPIOPinWrite(LED_TEMPO, 0x00);

	// Configure the DAC output timer - Timer 0
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
	ulDACPeriod = (ROM_SysCtlClockGet() / 44100); // 44.1 kHz for DAC samples
	ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, ulDACPeriod);
	// Set up interrupt
	ROM_IntPrioritySet(INT_TIMER0A, 0x00);
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
	ROM_IntPrioritySet(INT_TIMER1A, 0x40);
	ROM_IntEnable(INT_TIMER1A);
	ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	// Enable Timer
	ROM_TimerEnable(TIMER1_BASE, TIMER_A);

	// Configure the Tempo timer - Timer 2
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
	ROM_TimerConfigure(TIMER2_BASE, TIMER_CFG_PERIODIC);
	ulTempoPeriod = (ROM_SysCtlClockGet() / TEMPO_FREQUENCY);
	ROM_TimerLoadSet(TIMER2_BASE, TIMER_A, ulTempoPeriod);
	// Set up interrupt
	ROM_IntPrioritySet(INT_TIMER2A, 0x20);
	ROM_IntEnable(INT_TIMER2A);
	ROM_TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
	// Enable Timer
	ROM_TimerEnable(TIMER2_BASE, TIMER_A);

	// Configure the poll timer
	ulPollPeriod = (ROM_SysCtlClockGet() / POLL_FREQUENCY);
	ROM_TimerLoadSet(TIMER2_BASE, TIMER_B, ulPollPeriod);
	// Set up interrupt
	ROM_IntPrioritySet(INT_TIMER2B, 0x20);
	ROM_IntEnable(INT_TIMER2B);
	ROM_TimerIntEnable(TIMER2_BASE, TIMER_TIMB_TIMEOUT);
	// Enable Timer
	ROM_TimerEnable(TIMER2_BASE, TIMER_B);

}

void timer0_int_handler(void) {
	// Clear the interrupt flag
	ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	// Pass the next value to the DAC
	dac_write(*readPtr);
	readPtr++;

	if (readPtr == startPtr + BUFFER_SIZE)
	{
		readPtr = startPtr;
	}

//	// Debugging
//	if (g_ulTimeStamp % 100 == 0)
//	{
//		if (testTimer0)
//		{
//			UARTprintf("Timer 0 is firing\n");
//			testTimer0 = 0;
//		}
//	}
//	else
//	{
//		testTimer0 = 1;
//	}
}

void timer1_int_handler(void) {
	// Clear the interrupt flag
	ROM_TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

	//btn_pollRow();


	uint8_t doLoad = 0;

	if ((readPtr < (startPtr+PKT_SIZE)) && readPoint == 0)
	{
		doLoad = 1;
		readPoint = 1;
	}
	else if ((readPtr >= (startPtr+PKT_SIZE)) && readPoint == 1)
	{
		writePtr = startPtr;
		doLoad = 1;
		readPoint = 0;
	}

	if (doLoad)
	{
		int i;
		uint16_t pkt1[PKT_SIZE];
		uint16_t pkt2[PKT_SIZE];
		uint16_t finalPkt[PKT_SIZE];

		// Initialise output packets
		for (i = 0; i < PKT_SIZE; i++)
		{
			pkt1[i] = 0x8000;
			pkt2[i] = 0x8000;
			finalPkt[i] = 0x8000;
		}

		// Retrieve the current 2 audio streams
		for (i = 0; i < 16; i++)
		{
			uint16_t bitMask = 0 | (1 << i);
			uint8_t fileEnded = 0;

			if (!!(pressed & bitMask))
			{
				// Button is pressed
				// NEED ORDER CHECK ON ALL OF THESE
				FIL file;
				sdcard_openFile(&file, i);
				sdcard_readPacket(&file, i, &finalPkt[0]);
				sdcard_closeFile(&file);
				playing = playing | bitMask;
				//UARTprintf("READING!, playing = 0x%X\n", playing);
			}
			else
			{
				// Button is not pressed, but may still be playing
				if (!!(playing & bitMask) && !!(latchHold & bitMask))
				{
					// File is latched, need to finish playback
					FIL file;
					sdcard_openFile(&file, i);
					fileEnded = sdcard_readPacket(&file, i, &finalPkt[0]);
					sdcard_closeFile(&file);
					//UARTprintf("FILLING!, WL = %d, playing = 0x%X, fe = %d\n", whereLastPress[i], playing, fileEnded);
					//UARTprintf("FILLING, read=%d, write=%d, ts=%d\n", (readPtr - startPtr), (writePtr - startPtr), g_ulTimeStamp);
					if (fileEnded)
					{
						// File has completed playback, stop it playing next time!
						// Should make sure this bitbashing checks out
						playing = playing & ~bitMask;
						whereLastPress[i] = 0;

						// Debugging
						//UARTprintf("File %d ended!\n", i);

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
		}

		// DEBUGGING - unpress first button
		pressed = 0x00;
		//playing = 0x00;


		// At this point, we have 2 arrays of the most up to date audio streams
		// Need to apply convolution, then FX
		// NOT DONE YET, OBVIOUSLY. ONLY HAVE 1 PKT AT THE MOMENT

		// Load into the buffer
		for (i = 0; i < PKT_SIZE; i++)
		{
			*writePtr = finalPkt[i];
			writePtr++;
		}
	}

	pollCount++;
	if (pollCount == 9)
	{
		btn_pollRow();
		pollCount = 0;
	}


//	// Debugging
//	if (g_ulTimeStamp % 100 == 0)
//	{
//		if (testTimer1)
//		{
//			UARTprintf("Timer 1 is firing\n");
//			testTimer1 = 0;
//			// DEBUGGING
//			//playing = 0x01;
//		}
//	}
//	else
//	{
//		testTimer1 = 1;
//	}
}

void timer2_int_handler(void) {
	// Clear the interrupt flag
	ROM_TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);

	ulTempoTimestamp = (ulTempoTimestamp+1) % TEMPO_MULTIPLIER;

	if (ulTempoTimestamp == 0)
	{
		// TOGGLE THE TEMPO LED ON
		// UARTprintf("BEAT\n");
		ROM_GPIOPinWrite(LED_TEMPO, 0xFF);

	}
	else if (ulTempoTimestamp == (int)TEMPO_MULTIPLIER/8)
	{
		// TOGGLE THE TEMPO LED OFF
		ROM_GPIOPinWrite(LED_TEMPO, 0x00);
	}

	//btn_pollRow();
	update_LEDs();

	// Debugging
	if (g_ulTimeStamp % 100 == 0)
	{
		if (testTimer2)
		{
			testTimer2 = 0;
//			pressed = (1 << testSample);
//			//pressed = 0x2000;
//			testSample = (testSample+1)%16;
//			UARTprintf("Playing Sample %d\n", testSample, pressed, ulTempoTimestamp);
		}
	}
	else
	{
		testTimer2 = 1;
	}
}

