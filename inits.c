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
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom_map.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"

// Local Includes
#include "inits.h"


void init_gpio(void) {
	// Enable all ports
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// Enable NMI pins (PD7 + PF0)
	HWREG(GPIO_PORTD_BASE + 0x520) = 0x4C4F434B;
	HWREG(GPIO_PORTF_BASE + 0x520) = 0x4C4F434B;
	HWREG(GPIO_PORTD_BASE + 0x524) = 0x000000FF;
	HWREG(GPIO_PORTF_BASE + 0x524) = 0x000000FF;
	HWREG(GPIO_PORTD_BASE + 0x520) = 0x00000000;
	HWREG(GPIO_PORTF_BASE + 0x520) = 0x00000000;

}

void init_playback(void) {
	// Nothing here yet

}



