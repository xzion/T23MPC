///*
// * uart.c
// *
// * contains functions to handle uart communication
// *
// */
//
///* Library headers */
//#include "inc/hw_ints.h"
//#include "inc/hw_types.h"
//#include "inc/hw_memmap.h"
//#include "driverlib/debug.h"
//#include "driverlib/gpio.h"
//#include "driverlib/pin_map.h"
//#include "driverlib/rom.h"
//#include "driverlib/sysctl.h"
//
///* My file headers */
//#include "uart.h"
//
///*
// * Initialise UART0 (USB Debug COM Port)
// *
// * Enable peripheral, configure pins, enable UART
// * Initialises UART at default settings of 115200, 8-N-1
// */
//void init_uart(void) {
//	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
//	ROM_GPIOPinConfigure(GPIO_PA0_U0RX | GPIO_PA1_U0TX);
//	ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
//	UARTStdioInit(0);
//
//	UARTprintf("%c******* STARTING ********\n", 0x0C);
//}
