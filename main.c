/*
 * main.c - Initialisation and core loops
 *
 * Created on:	07/05/2013
 * Author: 		Coen McClelland
 * ENGG4810:	Team 23
 *
 */

// Library Includes
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "third_party/fatfs/src/diskio.h"

// Local Includes
#include "inits.h"

// Global Variables
volatile unsigned long g_ulTimeStamp = 0; // Time since boot in 10ms increments
extern unsigned long g_ulIdleTimeout;

/* SysTick Handler
 *
 * - Required by FatFS for SD card timings
 * - On a 10ms interrupt
 */
void SysTickHandler(void) {
	// Alert FatFS and increment the timestamp
    disk_timerproc();
    ++g_ulTimeStamp;
    if(g_ulIdleTimeout > 0) {
    	g_ulIdleTimeout--;
    }
}

/* Main function
 *
 * - Initialise device and any global variables
 * - Enable Interrupts
 * - Start endless loop
 */
void main(void) {
	// Initialise the device clock to 80MHz
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN );

	// Enable SysTick for FatFS at 10ms intervals
	ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 100);
	ROM_SysTickEnable();
	ROM_SysTickIntEnable();

	// Enable Global interrupts
	ROM_IntMasterEnable();

	// Enable floating point arithmetic unit, but disable stacking
//	ROM_FPUEnable();
//	ROM_FPUStackingDisable();

	// Initialise GPIO

	// Initialise GPIO Expander

	// Initialise Buttons

	// Initialise ADC

	// Initialise DAC

	// Initialise SD Card

#if DEBUG
	// Initialise UART for debugging

#endif

	/* if (GPIO Mode Pin = MPC Mode) {
	 * 	// Initialise MPC variables (logging, pressed, latchhold)
	 * }
	 */


	for(;;) {
		// Endless Loop
	}
}
