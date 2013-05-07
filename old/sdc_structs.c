/*
 * sdc_structs.c
 *
 * contains data structures for the MSC driver
 */

/* Library headers */
#include "inc/hw_types.h"
#include "third_party/fatfs/src/ff.h"
#include "usblib/usblib.h"
#include "usblib/usb-ids.h"
#include "usblib/device/usbdevice.h"
#include "usblib/device/usbdmsc.h"

/* My file headers */
#include "sdc.h"
#include "sdc_structs.h"

/* Languages supported */
const unsigned char g_pLangDescriptor[] = {
    4,
    USB_DTYPE_STRING,
    USBShort(USB_LANG_EN_US)
};
/* Manufacturer string */
const unsigned char g_pManufacturerString[] = {
	(17 + 1) * 2,
	USB_DTYPE_STRING,
	'T', 0, 'e', 0, 'x', 0, 'a', 0, 's', 0, ' ', 0, 'I', 0, 'n', 0, 's', 0,
	't', 0, 'r', 0, 'u', 0, 'm', 0, 'e', 0, 'n', 0, 't', 0, 's', 0,
};
/* Product string */
const unsigned char g_pProductString[] = {
	(19 + 1) * 2,
	USB_DTYPE_STRING,
	'M', 0, 'a', 0, 's', 0, 's', 0, ' ', 0, 'S', 0, 't', 0, 'o', 0, 'r', 0,
	'a', 0, 'g', 0, 'e', 0, ' ', 0, 'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0,
	'e', 0
};
/* Serial number */
const unsigned char g_pSerialNumberString[] = {
    (8 + 1) * 2,
    USB_DTYPE_STRING,
    '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0
};
/* Data interface description string */
const unsigned char g_pDataInterfaceString[] = {
    (19 + 1) * 2,
    USB_DTYPE_STRING,
    'B', 0, 'u', 0, 'l', 0, 'k', 0, ' ', 0, 'D', 0, 'a', 0, 't', 0,
    'a', 0, ' ', 0, 'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0,
    'a', 0, 'c', 0, 'e', 0
};
/* Configuration description string */
const unsigned char g_pConfigString[] = {
    (23 + 1) * 2,
    USB_DTYPE_STRING,
    'B', 0, 'u', 0, 'l', 0, 'k', 0, ' ', 0, 'D', 0, 'a', 0, 't', 0,
    'a', 0, ' ', 0, 'C', 0, 'o', 0, 'n', 0, 'f', 0, 'i', 0, 'g', 0,
    'u', 0, 'r', 0, 'a', 0, 't', 0, 'i', 0, 'o', 0, 'n', 0
};
/* Descriptor string table */
const unsigned char * const g_pStringDescriptors[] = {
    g_pLangDescriptor,
    g_pManufacturerString,
    g_pProductString,
    g_pSerialNumberString,
    g_pDataInterfaceString,
    g_pConfigString
};
#define NUM_STRING_DESCRIPTORS (sizeof(g_pStringDescriptors) / sizeof(unsigned char *))

/* Bulk device initialisation and customisation structures */
tMSCInstance g_sMSCInstance;

const tUSBDMSCDevice g_sMSCDevice = {
	USB_VID_STELLARIS, /* Vendor ID from USB-IF */
	USB_PID_MSC, /* Product ID for this device */
	"TI      ", /* 8-byte vendor string */
	"Mass Storage    ", /* 16-byte product string */
	"1.00", /* 4-byte version string */
	500, /* Max power consumption in mA */
	USB_CONF_ATTR_SELF_PWR, /* Device is self-powered */
	g_pStringDescriptors, /* Pointer to the string descriptor array */
	NUM_STRING_DESCRIPTORS, /* Number of descriptors */
	{ /* Structure holding access functions for SD card */
		USBDMSCStorageOpen,
		USBDMSCStorageClose,
		USBDMSCStorageRead,
		USBDMSCStorageWrite,
		USBDMSCStorageNumBlocks
	},
	USBDMSCEventCallback, /* Call-back function */
	&g_sMSCInstance /* Pointer to the private instance data */
};

#define MSC_BUFFER_SIZE	512
