/*
 * lfo.h
 *
 *  Created on: 27/05/2013
 *      Author: Coen
 */

#ifndef LFO_H_
#define LFO_H_

extern void lfo_init(void);
extern void lfo_setFreq(uint8_t adcVal);
extern void lfo_increment(void);
extern uint8_t lfo_getLFOVal(void);

#endif /* LFO_H_ */
