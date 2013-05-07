/*
 * dials.h
 *
 * contains function prototypes for adc.c
 *
 */

#ifndef DIALS_H_
#define DIALS_H_

extern void init_adc(void);
extern void adc_trigger(void);
long adc_data_get(unsigned long* ulData);

#endif /* DIALS_H_ */
