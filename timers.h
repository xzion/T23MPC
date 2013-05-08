/*
 * inits.h
 *
 *  Created on: 07/05/2013
 *  Author: 	Coen McClelland
 *
 * Contains initialisation function prototypes
 *
 */

#ifndef TIMERS_H_
#define TIMERS_H_

/* Init_Playback
 *
 * Initialises playback variables from cfg.dat on the SD
 * This includes loop intervals, latch/hold status, FX knobs and tempo
 */
extern void init_playback(void);

/* Init_Timers
 *
 * Initialises 2 timers used for DAC output (audio playback)
 * and updating the output buffer.
 */
extern void init_timers(void);

extern void timer0_int_handler(void);

extern void timer1_int_handler(void);

#endif /* TIMERS_H_ */
