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
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "third_party/fatfs/src/diskio.h"
#include "driverlib/gpio.h"
#include <stdint.h>

// Local Includes
#include "timers.h"
#include "sdcard.h"
#include "uart.h"
#include "dac.h"
#include "btn.h"

// Definitions


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

	// Initialise GPIO - All ports enabled
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Unlock NMI pins for GPIO usage (PD7 + PF0)
	HWREG(GPIO_PORTD_BASE + 0x520) = 0x4C4F434B;
	HWREG(GPIO_PORTF_BASE + 0x520) = 0x4C4F434B;
	HWREG(GPIO_PORTD_BASE + 0x524) = 0x000000FF;
	HWREG(GPIO_PORTF_BASE + 0x524) = 0x000000FF;
	HWREG(GPIO_PORTD_BASE + 0x520) = 0x00000000;
	HWREG(GPIO_PORTF_BASE + 0x520) = 0x00000000;

	// Initialise GPIO Expander (probably not happening)

	// Initialise Buttons
	btn_init();

	// Initialise ADC

	// Initialise DAC
	dac_init();

	// Initialise SD Card
	sdcard_init();

	// Initialise UART for debugging
	uart_init();

	// Initialise Timers
	timers_init();




	for(;;) {
		// Endless Loop
	}
}
