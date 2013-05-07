/*
 * sdc.c
 *
 * contains functions to interface with the SD card
 * Based on sd_card.c - part of revision 9453 of the EK-LM4F232 Firmware Package
 *
 */

/* Library headers */
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/udma.h"
#include "third_party/fatfs/src/ff.h"
#include "third_party/fatfs/src/diskio.h"
#include "usblib/usblib.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdmsc.h"

/* My file headers */
#include "sdc.h"
#include "sdc_structs.h"

/* Definitions */
#define FRESULT_ENTRY(f)		{ (f), (#f) } /* Macro to make it easier to add FRESULTs to table */
#define NUM_FRESULT_CODES		(sizeof(pxFresultStrings) / sizeof(tFresultString)) /* Macro that stores the number of FRESULT codes */
#define SDC_PERIPH_SSI			SYSCTL_PERIPH_SSI1
#define SDCARD_PRESENT      	0x00000001
#define SDCARD_IN_USE       	0x00000002
#define USBMSC_ACTIVITY_TIMEOUT	300 /* Timeout in 10ms of the USB connection */

/* TypeDefs */
#if DEBUG
/* Struct to hold a mapping between a numerical error code and the string representation - DEBUG ONLY */
typedef struct {
    FRESULT fresult;
    char *pcResultStr;
} tFresultString;
#endif /* DEBUG */

/* Variables */
tBoolean g_bSDCardPresent = FALSE;
extern volatile unsigned long g_ulTimeStamp;
unsigned long g_ulIdleTimeout = 0;
struct {
    unsigned long ulFlags;
} g_sDriveInformation;
static FATFS xFatFs; /* Structure used by FatFS - instance of FS */
volatile enum {
    MSC_DEV_DISCONNECTED, /* Unconfigured */
    MSC_DEV_CONNECTED, /* Connected but not yet fully enumerated */
    MSC_DEV_IDLE, /* Connected and enumerated but not handling a command */
    MSC_DEV_READ, /* Currently reading from the SD */
    MSC_DEV_WRITE /* Currently writing to the SD */
} g_eMSCState;
/* DMA Control structure table */
#pragma DATA_ALIGN(sDMAControlTable, 1024)
tDMAControlTable sDMAControlTable[64];
#if DEBUG
/* Table of error string representations - DEBUG ONLY */
tFresultString pxFresultStrings[] = {
    FRESULT_ENTRY(FR_OK),
    FRESULT_ENTRY(FR_NOT_READY),
    FRESULT_ENTRY(FR_NO_FILE),
    FRESULT_ENTRY(FR_NO_PATH),
    FRESULT_ENTRY(FR_INVALID_NAME),
    FRESULT_ENTRY(FR_INVALID_DRIVE),
    FRESULT_ENTRY(FR_DENIED),
    FRESULT_ENTRY(FR_EXIST),
    FRESULT_ENTRY(FR_RW_ERROR),
    FRESULT_ENTRY(FR_WRITE_PROTECTED),
    FRESULT_ENTRY(FR_NOT_ENABLED),
    FRESULT_ENTRY(FR_NO_FILESYSTEM),
    FRESULT_ENTRY(FR_INVALID_OBJECT),
    FRESULT_ENTRY(FR_MKFS_ABORTED)
};
#endif /* DEBUG */


#if DEBUG
/*
 * Returns the string representation of an error code
 *
 * DEBUG ONLY
 */
const char* string_from_fresult(FRESULT xFresult) {
    unsigned char i;
    /* Loop to find the resultant string */
    for(i = 0; i < NUM_FRESULT_CODES; ++i) {
        if(pxFresultStrings[i].fresult == xFresult) {
            return(pxFresultStrings[i].pcResultStr);
        }
    }
    /* Couldn't find it */
    return("UNKNOWN ERROR CODE");
}
#endif /* DEBUG */

/*
 * Error routine called by USB library
 */
#ifdef DEBUG
void __error__(char *pcFilename, unsigned long ulLine) {
	UARTprintf("HEY! We got an error :(\n");
}
#endif

/*
 * Initialises FatFS and SD card stuff
 *
 * returns true if successful, false if not
 */
tBoolean init_sdc(void) {
	/* Enable SSI1 for use with SDC */
	ROM_SysCtlPeripheralEnable(SDC_PERIPH_SSI);
	/* Mount the SD card using logical system 0 */
	FRESULT FResult = f_mount(0, &xFatFs);
	if(FResult != (FRESULT)FR_OK) {
#if DEBUG
		UARTprintf("f_mount error: %s\n", string_from_fresult(FResult));
#endif /* DEBUG */
		g_bSDCardPresent = FALSE;
		return FALSE;
	}
	g_bSDCardPresent = TRUE;
	init_sdc_msc();
	return TRUE;
}

/*
 * Opens a file and gets the file pointer
 *
 * Takes the file pointer and sample index (0xFF if config file)
 */
tBoolean sdc_open(FIL *pxFile, unsigned char ucIndex) {
	while(!sdc_available());
	char pcFN[12] = "/config.txt";
	if(ucIndex != 0xFF) {
		pcFN[1] = 's';
		pcFN[2] = 'm';
		pcFN[3] = 'p';
		pcFN[4] = 'l';
		pcFN[5] = (ucIndex + 1) / 10 + '0';
		pcFN[6] = (ucIndex + 1) % 10 + '0';
		pcFN[7] = 0;
	}
	/* Attempt to open file */
	FRESULT FResult = f_open(pxFile, pcFN, FA_READ | FA_OPEN_EXISTING);
	if(FResult != FR_OK) {
#if DEBUG
		UARTprintf("f_open error: %s\n", string_from_fresult(FResult));
#endif /* DEBUG */
		return FALSE;
	}
	return TRUE;
}

/*
 * Closes a file pointer
 */
tBoolean sdc_close(FIL *pxFile) {
	while(!sdc_available());
	/* Attempt to close file */
	FRESULT FResult = f_close(pxFile);
	if(FResult != FR_OK) {
#if DEBUG
		UARTprintf("f_close error: %s\n", string_from_fresult(FResult));
#endif /* DEBUG */
		return FALSE;
	}
	return TRUE;
}

/*
 * Reads the next block of data from a file pointer
 *
 * Returns true if successful
 * Takes the file pointer, a pre-allocated buffer and the # of bytes to read
 */
tBoolean sdc_read(FIL *pxFile, void *pvBuffer, unsigned short usBytesToRead) {
	while(!sdc_available());
	unsigned short usBytesRead;
	/* Attempt to read from file */
	FRESULT FResult = f_read(pxFile, pvBuffer, usBytesToRead, &usBytesRead);
	if(FResult != FR_OK) {
#if DEBUG
		UARTprintf("f_read error: %s\n", string_from_fresult(FResult));
#endif /* DEBUG */
		return FALSE;
	}
	return TRUE;
}

/*
 * Resets a file to point to the beginning
 *
 * Returns true if successful
 */
tBoolean sdc_reset(FIL *pxFile) {
	FRESULT FResult = f_lseek(pxFile, 0);
	if(FResult != (FRESULT)FR_OK) {
#if DEBUG
		UARTprintf("f_lseek error: %s\n", string_from_fresult(FResult));
#endif /* DEBUG */
		return FALSE;
	}
	return TRUE;
}

/*
 * Initialises the MSC interface
 */
void init_sdc_msc(void) {
	g_ulIdleTimeout = 0;
	/* Configure uDMA */
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	SysCtlDelay(10);
	ROM_uDMAControlBaseSet(&sDMAControlTable[0]);
	ROM_uDMAEnable();
	/* Enable USB controller and configure pins */
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_USB0);
	ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	disk_initialize(0);
	/* Pass device info to library */
	USBDMSCInit(0, (tUSBDMSCDevice *)&g_sMSCDevice);
}

/*
 * Checks the state of the MSC driver
 *
 * Returns TRUE if it is idle or disconnected, FALSE if in use
 * Use this to check if the PC is accessing the SD
 */
tBoolean sdc_available(void) {
	switch(g_eMSCState) {
		case MSC_DEV_READ:
			if(g_ulIdleTimeout == 0) {
				g_eMSCState = MSC_DEV_IDLE;
				return TRUE;
			}
			return FALSE;
		case MSC_DEV_WRITE:
			if(g_ulIdleTimeout == 0) {
				g_eMSCState = MSC_DEV_IDLE;
				return TRUE;
			}
			return FALSE;
		case MSC_DEV_DISCONNECTED:
			return TRUE;
		case MSC_DEV_IDLE:
			return TRUE;
		default:
			return FALSE;
	}
}

/*
 * Handles bulk driver notifications related to the RX channel
 *
 * This simply acknowledges the data transfer, allowing the PC to send more data
 */
unsigned long RxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData) {
	/* We've received some data */
    return 0;
}

/*
 * Handles bulk driver notifications related to the TX channel
 *
 * This is simply an acknowledgement of the data transfer, allowing us to send more data
 */
unsigned long TxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData) {
	/* We sent some data */
    return 0;
}

/*
 * Call-back function for event handling
 *
 * Is essentially just used to monitor the PC
 * Called by USB library, no manual calls to be made
 */
unsigned long USBDMSCEventCallback(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgParam, void *pvMsgData) {
	/* Reset the timeout when something happens */
	g_ulIdleTimeout = USBMSC_ACTIVITY_TIMEOUT;
	/* Act on the event */
	switch(ulEvent) {
	case USBD_MSC_EVENT_WRITING:
		g_eMSCState = MSC_DEV_WRITE;
		break;
	case USBD_MSC_EVENT_READING:
		g_eMSCState = MSC_DEV_READ;
		break;
	case USB_EVENT_DISCONNECTED:
		g_eMSCState = MSC_DEV_DISCONNECTED;
		break;
	case USB_EVENT_CONNECTED:
		g_eMSCState = MSC_DEV_IDLE;
		break;
	default:
		g_eMSCState = MSC_DEV_IDLE;
		break;
	}
	return 0;
}

/*
 * Function to prepare SD card for USB
 *
 * Assumes f_mount has already been called
 * Called by USB library, no manual calls to be made
 */
void *USBDMSCStorageOpen(unsigned long ulDrive) {
    if(ulDrive != 0) return 0;
    /* Return if already in use or not present */
    if(!g_bSDCardPresent || (g_sDriveInformation.ulFlags & SDCARD_IN_USE)) {
    	return 0;
    }
	/* Card is present and in use */
	g_sDriveInformation.ulFlags = SDCARD_PRESENT | SDCARD_IN_USE;
    return((void *)&g_sDriveInformation);
}

/*
 * Closes the drive in use by the MSC
 *
 * Called by USB library, no manual calls to be made
 */
void *USBDMSCStorageClose(void * pvDrive) {
    if(pvDrive == 0) {
    	return (void*)1;
    }
    /* Clear all flags */
    g_sDriveInformation.ulFlags = 0;
    /* Unmount the card */
    if(f_mount(0, 0) != (FRESULT)FR_OK) {
    	return (void*)1;
    }
    return (void*)0;
}

/*
 * Reads data in 512-byte blocks
 *
 * Returns the number of bytes read
 * Called by USB library, no manual calls to be made
 */
unsigned long USBDMSCStorageRead(void * pvDrive, unsigned char *pucData, unsigned long ulSector, unsigned long ulNumBlocks) {
    if(pvDrive == 0) {
    	return 0;
    }
    if(disk_read(0, pucData, ulSector, ulNumBlocks) == RES_OK) {
        return(ulNumBlocks * 512);
    }
    return 0;
}

/*
 * Writes data in 512-byte blocks
 *
 * Returns the number of bytes written
 * Called by USB library, no manual calls to be made
 */
unsigned long USBDMSCStorageWrite(void * pvDrive, unsigned char *pucData, unsigned long ulSector, unsigned long ulNumBlocks) {
    if(pvDrive == 0) {
    	return 0;
    }
    if(disk_write(0, pucData, ulSector, ulNumBlocks) == RES_OK) {
        return(ulNumBlocks * 512);
    }
    return 0;
}

/*
 * Returns the number of blocks present on the device
 *
 * Called by USB library, no manual calls to be made
 */
unsigned long USBDMSCStorageNumBlocks(void * pvDrive) {
    unsigned long ulSectorCount;
    disk_ioctl(0, GET_SECTOR_COUNT, &ulSectorCount);
    return(ulSectorCount);
}
