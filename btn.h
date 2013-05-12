/*
 * btn.h
 *
 *  Created on: 12/05/2013
 *      Author: Coen
 */

#ifndef BTN_H_
#define BTN_H_

extern void btn_init(void);
extern void btn_pollRow(void);
extern uint8_t debounce(uint8_t btnNum, uint8_t rawInput);
extern void update_LEDs(void);

#endif /* BTN_H_ */
