/*
 * dac.c
 *
 *  Created on: 08/05/2013
 *  Author: 	Coen McClelland
 */

// Library Includes
#include "inc/hw_ints.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include <stdint.h>

// Local Includes
#include "dac.h"

void init_dac(void) {
	// Enable SSI0 for the SD Card
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
	// Configure Pins
	ROM_GPIOPinConfigure(GPIO_PB4_SSI2CLK);
	ROM_GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_4);
	ROM_GPIOPinConfigure(GPIO_PB5_SSI2FSS);
	ROM_GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_5);
	ROM_GPIOPinConfigure(GPIO_PB7_SSI2TX);
	ROM_GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_7);
	// Clock speed for DAC
	ROM_SSIConfigSetExpClk(SSI2_BASE, ROM_SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 16);
	// Enable SSI
	ROM_SSIEnable(SSI2_BASE);
}

void dac_write(uint16_t sample) {

	// Construct the correct output bits
	// 0x3000 = command data
	// Remaining 12 bits = audio data
	// Will be update to 16 bit
	uint16_t dacOutput = (sample & 0x0FFF) | 0x3000;

	ROM_SSIDataPut(SSI2_BASE, dacOutput);

	while(ROM_SSIBusy(SSI2_BASE));
}
