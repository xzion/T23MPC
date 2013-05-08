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


/* Init_Timers
 *
 * Initialises 2 timers used for DAC output (audio playback)
 * and updating the output buffer.
 */
extern void init_timers(void);

extern void timer0_int_handler(void);

extern void timer1_int_handler(void);

extern void timer2_int_handler(void);

#endif /* TIMERS_H_ */
