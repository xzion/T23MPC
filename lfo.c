/*
 * lfo.c
 *
 *  Created on: 27/05/2013
 *      Author: Coen
 */

// Library includes
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include <stdint.h>

// Local Includes
#include "lfo.h"

// Variables
float LFOvalue;
uint16_t counter;
uint16_t freq;
uint8_t increasing;


void lfo_init(void) {
	LFOvalue = 0;
	increasing = 1;
	freq = 9999;
	counter = 0;
}

void lfo_setFreq(uint8_t adcVal) {
	freq = adcVal*3465/128+35;
}

void lfo_increment(void) {
	counter++;
	if (counter % freq == 0)
	{
		// Change the LFO value
		if (increasing)
		{
			LFOvalue++;
			if (LFOvalue == 127)
			{
				increasing = 0;
			}
		}
		else
		{
			LFOvalue--;
			if (LFOvalue == 0)
			{
				increasing = 1;
			}
		}
	}
}
uint8_t lfo_getLFOVal(void) {
	return LFOvalue;
}


/* TODO:
 *  - Make an extern for LFOEnabled
 *  - Enable lfo with FN+2
 *  - Call increment from DAC timer
 *  - Call getVal in FX if enabled
 */
