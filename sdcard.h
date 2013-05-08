/*
 * sdcard.h
 *
 *  Created on: 08/05/2013
 *  Author: 	Coen McClelland
 */

#ifndef SDCARD_H_
#define SDCARD_H_

#include "third_party/fatfs/src/ff.h"

extern void init_sdcard(void);
extern void sdcard_openFile(FIL * file);
extern char sdcard_readByte(FIL * file);
extern void sdcard_readPacket(FIL * file, uint16_t * pktPtr, uint16_t pktLen);
extern void sdcard_closeFile(FIL * file);

#endif /* SDCARD_H_ */
