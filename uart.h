/*
 * uart.h
 *
 *  Created on: 08/05/2013
 *      Author: Coen
 */

#ifndef UART_H_
#define UART_H_

#include "utils/uartstdio.h"

/*
 * Initialise UART0 for debugging
 * Default settings 115200 baud, 8-N-1
 *
 */
extern void uart_init(void);

#endif /* UART_H_ */
