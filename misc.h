/*
 * inits.h
 *
 *  Created on: 07/05/2013
 *  Author: 	Coen McClelland
 *
 * Contains initialisation function prototypes
 *
 */

#ifndef INITS_H_
#define INITS_H_

/* Init_GPIO
 *
 * Initialises the device GPIO Headers
 */
extern void init_gpio(void);

/* Init_Playback
 *
 * Initialises playback variables from cfg.dat on the SD
 * This includes loop intervals, latch/hold status, FX knobs and tempo
 */
extern void init_playback(void);

#endif /* INITS_H_ */
