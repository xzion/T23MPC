///*
// * sdc.h
// *
// * contains functions prototypes for sdc.c
// */
//
//#ifndef SDC_H_
//#define SDC_H_
//
//#ifndef f_eof
//#define f_eof(fp) (((fp)->fptr) == ((fp)->fsize) ? 1 : 0)
//#endif
//#ifndef f_tell
//#define f_tell(fp) ((fp)->fptr)
//#endif
//
//#include "third_party/fatfs/src/ff.h"
//extern const char* string_from_fresult(FRESULT xFresult);
//extern tBoolean init_sdc(void);
//extern tBoolean sdc_open(FIL *pxFile, unsigned char ucIndex);
//extern tBoolean sdc_close(FIL *pxFile);
//extern tBoolean sdc_read(FIL *pxFile, void *pvBuffer, unsigned short usBytesToRead);
//extern tBoolean sdc_reset(FIL *pxFile);
//extern void init_sdc_msc(void);
//extern tBoolean sdc_available(void);
//extern unsigned long RxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData);
//extern unsigned long TxHandler(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgValue, void *pvMsgData);
//extern unsigned long USBDMSCEventCallback(void *pvCBData, unsigned long ulEvent, unsigned long ulMsgParam, void *pvMsgData);
//extern void *USBDMSCStorageOpen(unsigned long ulDrive);
//extern void *USBDMSCStorageClose(void * pvDrive);
//extern unsigned long USBDMSCStorageRead(void * pvDrive, unsigned char *pucData, unsigned long ulSector, unsigned long ulNumBlocks);
//extern unsigned long USBDMSCStorageWrite(void * pvDrive, unsigned char *pucData, unsigned long ulSector, unsigned long ulNumBlocks);
//extern unsigned long USBDMSCStorageNumBlocks(void * pvDrive);
//
//#endif /* SDC_H_ */
