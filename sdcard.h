/*
 * sdcard.h
 *
 *  Created on: 08/05/2013
 *  Author: 	Coen McClelland
 */

#ifndef SDCARD_H_
#define SDCARD_H_

#include "third_party/fatfs/src/ff.h"

#define BUFFERUPDATE_FREQ			500  // Started with 100hz
#define PKT_SIZE					512         //44100/BUFFERUPDATE_FREQ

/* Configure_Playback
 *
 * Initialises playback variables from cfg.dat on the SD
 * This includes loop intervals, latch/hold status, FX knobs and tempo
 */
extern void configure_playback(void);
extern void sdcard_init(void);
extern void sdcard_openFile(FIL * file, int ID);
extern void sdcard_resetFile(FIL * file);
extern char sdcard_readByte(FIL * file);
extern uint8_t sdcard_readPacket(FIL * file, uint8_t buttonNumber, uint16_t * pktPtr);
extern void sdcard_closeFile(FIL * file);

#endif /* SDCARD_H_ */
