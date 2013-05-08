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

// Definitions
#define BUFFERUPDATE_FREQ			100

// Variables
uint8_t testTimer0 = 1;
uint8_t testTimer1 = 1;
extern volatile unsigned long g_ulTimeStamp;

void init_playback(void) {
	// Nothing here yet

}

void init_timers(void) {
	unsigned long ulDACPeriod, ulPktPeriod;

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

}

void timer0_int_handler(void) {
	// Clear the interrupt flag
	ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);







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






	// Debugging
	if (g_ulTimeStamp % 100 == 0 && testTimer1)
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



