/*
 * buttons.h
 *
 * contains function prototypes for buttons.c
 *
 */
#ifndef BUTTONS_H_
#define BUTTONS_H_

extern unsigned short g_usLooping;
extern void init_buttons(void);
extern void scan_buttons(unsigned char* pucButtons, unsigned long* pulDebounce);
extern void set_leds(unsigned char* pucLeds, unsigned char ucLoop);
extern char process_buttons(unsigned char* pucButtons, unsigned char* pucLeds);

#endif /* BUTTONS_H_ */
