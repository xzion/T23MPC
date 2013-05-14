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
extern volatile unsigned long g_ulTimeStamp;
unsigned long g_ulIdleTimeout = 0;
const char fileArr[17][7] = {"00.dat", "01.dat", "02.dat", "03.dat", "04.dat", "05.dat", "06.dat", "07.dat", "08.dat", "09.dat", "10.dat", "11.dat", "12.dat", "13.dat", "14.dat", "15.dat", "config"};

uint32_t lastPressTs[16];
uint32_t whereLastPress[16];
uint8_t btnLoopMode[16];
uint16_t loopMod;
uint16_t pressed;
uint16_t playing;
uint16_t looping;
uint16_t latchHold;
uint8_t FX1mode;
uint8_t FX2mode;
uint8_t tempo;




void configure_playback(void) {
	int i;
	// Nothing here yet
	pressed = 0x0000; // FIRST BUTTON PRESSED, FOR DEBUGGING
	playing = 0x0000;
	latchHold = 0xFFFF;
	looping = 0x0000;
	FX1mode = 0x00;
	FX2mode = 0x01;
	tempo = 0x80;
	loopMod = 0xFFFF;

	// Initialise pointer positions
	for (i = 0; i < 16; i++)
	{
		whereLastPress[i] = 0;
		btnLoopMode[i] = 0x01;
		//UARTprintf("%d filestring %s\n", i, &fileArr[i][0]);
	}
}

/* Init_sdcard
 *
 * Initialise FatFS and SD Card
 */
void sdcard_init(void) {
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

	configure_playback();

}

void sdcard_openFile(FIL * file, int ID) {
	FRESULT fres = f_open(file, &fileArr[ID][0], FA_OPEN_EXISTING | FA_READ);

	if (fres != (FRESULT)FR_OK) {
		UARTprintf("f_open result: %d, name = %s, ID = %d\r\n", fres, fileArr[ID][0], ID);
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

uint8_t sdcard_readPacket(FIL * file, uint8_t buttonNumber, uint16_t * pktPtr) {
	uint8_t fileEnded = 0;
	int i;
	if (whereLastPress[buttonNumber] < file->fsize)
	{
		// Seek to the correct position
		FRESULT fres = f_lseek(file, whereLastPress[buttonNumber]);
		if (fres != (FRESULT)FR_OK)
		{
			// FatFS error
			UARTprintf("f_lseek result: %d\r\n", fres);
		}

		// Check if we can read the entire sample at once!
		// NB: pktLen needs to be less than 512 i think?

		if ((whereLastPress[buttonNumber] + PKT_SIZE*2) < file->fsize)
		{
			// We can read the whole packet!
			uint16_t bytesRead = 0;
			uint8_t readArray[PKT_SIZE*2];

			FRESULT fres = f_read(file, &readArray[0], PKT_SIZE*2, &bytesRead);
			if (fres != (FRESULT)FR_OK)
			{
				// FatFS error
				UARTprintf("f_read result: %d\r\n", fres);
				return 1;
			}
			else if (bytesRead != PKT_SIZE*2)
			{
				// Incorrect read, possible EOF
				UARTprintf("f_read did not read the correct number of bytes, read = %d, wanted = %d\n", bytesRead, PKT_SIZE*2);
				return 1;
			}

			// We've got the whole buffer OK, process it
			for (i = 0; i < PKT_SIZE; i++)
			{
				uint16_t readSample = readArray[2*i+1] | (readArray[2*i] << 8);
				*pktPtr = readSample;
				pktPtr++;
				whereLastPress[buttonNumber] += 2;
//				if (i > PKT_SIZE-30)
//				{
//					UARTprintf("%d - 0x%X\n", i, readSample);
//				}

			}

		}
		else
		{
			// Only a small portion of the sample is left
			uint32_t remainingShorts = PKT_SIZE - ((whereLastPress[buttonNumber] + PKT_SIZE*2) - file->fsize)/2;

			// Read what's left
			uint16_t bytesRead = 0;
			uint8_t readArray[PKT_SIZE*2];

			FRESULT fres = f_read(file, &readArray[0], remainingShorts*2, &bytesRead);
			if (fres != (FRESULT)FR_OK)
			{
				// FatFS error
				UARTprintf("f_read result: %d\r\n", fres);
				return 1;
			}
			else if (bytesRead != remainingShorts*2)
			{
				// Incorrect read, possible EOF
				UARTprintf("f_read remaining did not read the correct number of bytes, read = %d, wanted = %d\n", bytesRead, remainingShorts*2);
				return 1;
			}

			// Process the data for the output buffer
			for (i = 0; i < remainingShorts; i++)
			{
				uint16_t readSample = readArray[2*i+1] | (readArray[2*i] << 8);
				*pktPtr = readSample;
				pktPtr++;
				whereLastPress[buttonNumber] += 2;
			}
			// Fill the rest with zeros
			for (; i < PKT_SIZE; i++)
			{
				*pktPtr = 0x0000;
				//*pktPtr = 0x8000;
				pktPtr++;
				whereLastPress[buttonNumber] += 2;
			}
			// Signal the the file is finished
			fileEnded = 1;
		}
	}
	else
	{
		// The file has run past the end
		// Someone is probably holding the button down
		fileEnded = 1;
	}


	return fileEnded;
}

void sdcard_closeFile(FIL * file) {
	FRESULT fres = f_close(file);

	if (fres != (FRESULT)FR_OK) {
		UARTprintf("f_close result: %d\r\n", fres);
	}
}


