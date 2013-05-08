/*
 * sdcard.c
 *
 *  Created on: 08/05/2013
 *  Author: 	Coen
 */


// Library Includes
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/udma.h"
#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"
#include "usblib/usblib.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdmsc.h"
#include <stdint.h>

// Local Includes
#include "sdcard.h"
#include "uart.h"



// Variables
static FATFS fs;



/* Init_sdcard
 *
 * Initialise FatFS and SD Card
 */
void init_sdcard(void) {
	// Enable SSI0 for the SD Card
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
	// Configure Pins
	ROM_GPIOPinConfigure(GPIO_PA4_SSI0RX);
	ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_4);
	ROM_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
	ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2);
	ROM_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
	ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_3);
	ROM_GPIOPinConfigure(GPIO_PA5_SSI0TX);
	ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5);

	FRESULT fres = f_mount(0, &fs);

	if (fres != (FRESULT)FR_OK) {
		UARTprintf("f_mount result: %d\r\n", fres);
	}

}

void sdcard_openFile(FIL * file) {
	FRESULT fres = f_open(file, "00.dat", FA_OPEN_EXISTING | FA_READ);

	if (fres != (FRESULT)FR_OK) {
		UARTprintf("f_open result: %d\r\n", fres);
	}

}

char sdcard_readByte(FIL * file) {
	char retChar;
	uint16_t bytesRead = 0;

	FRESULT fres = f_read(file, &retChar, 1, &bytesRead);

	if (fres != (FRESULT)FR_OK)
	{
		UARTprintf("f_read result: %d\r\n", fres);
	}
	else if (bytesRead != 1)
	{
		// Print error to uart
		UARTprintf("f_read did not read the correct number of bytes\r\n", fres);
	}

	return retChar;
}

void sdcard_readPacket(FIL * file, uint16_t * pktPtr, uint16_t pktLen) {

	int i;
	for (i = 0; i < pktLen; i++)
	{
		uint16_t bytesRead = 0;
		uint16_t readSample;
		uint8_t arr[2];

		FRESULT fres = f_read(file, &arr[0], 2, &bytesRead);

		if (fres != (FRESULT)FR_OK)
		{
			// FatFS error
			UARTprintf("f_read result: %d\r\n", fres);
		}
		else if (bytesRead != 2)
		{
			// Incorrect read, possible EOF
			UARTprintf("f_read did not read the correct number of bytes\r\n", fres);
		}
		else
		{
			// OK!
			readSample = arr[1] | (arr[0] << 8);
			*pktPtr = readSample;
			pktPtr++;
		}
	}
}

void sdcard_closeFile(FIL * file) {
	FRESULT fres = f_close(file);

	if (fres != (FRESULT)FR_OK) {
		UARTprintf("f_close result: %d\r\n", fres);
	}
}


