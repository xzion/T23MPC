/*
 * fx.h
 *
 *  Created on: 16/05/2013
 *      Author: Coen
 */

#ifndef FX_H_
#define FX_H_

extern void fx_init(void);
extern void fx_apply(uint16_t * pkt);
extern void lowpass(uint16_t * pkt);
extern void highpass(uint16_t * pkt);
extern void bandpass(uint16_t * pkt);
extern void bandstop(uint16_t * pkt);
extern void delay(uint16_t * pkt);
extern void echo(uint16_t * pkt);
extern void decimator(uint16_t * pkt);
extern void bitcrusher(uint16_t * pkt);
extern void bitwiseko(uint16_t * pkt);


#endif /* FX_H_ */
