///*
// * dac.c
// *
// * contains functions to communicate with DAC over SSI
// *
// */
//
///* Library headers */
//#include "inc/hw_ints.h"
//#include "inc/hw_ssi.h"
//#include "inc/hw_types.h"
//#include "inc/hw_memmap.h"
//#include "driverlib/gpio.h"
//#include "driverlib/interrupt.h"
//#include "driverlib/pin_map.h"
//#include "driverlib/rom.h"
//#include "driverlib/ssi.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/timer.h"
//
///* My file headers */
//#include "dac.h"
//
///* Definitions */
//#define SSI_DAC				SYSCTL_PERIPH_SSI2
//#define SSI_DAC_BASE		SSI2_BASE
//#define GPIO_DAC			SYSCTL_PERIPH_GPIOB
//#define GPIO_DAC_BASE		GPIO_PORTB_BASE
//#define SSI_CLK_DAC			GPIO_PB4_SSI2CLK
//#define SSI_FSS_DAC			GPIO_PB5_SSI2FSS
//#define SSI_TX_DAC			GPIO_PB7_SSI2TX
//#define GPIO_DAC_CLK		GPIO_PIN_4
//#define	GPIO_DAC_FSS		GPIO_PIN_5
//#define GPIO_DAC_TX			GPIO_PIN_7
//#define DAC_TIMER			SYSCTL_PERIPH_TIMER0
//#define DAC_TIMER_BASE		TIMER0_BASE
//#define DAC_TIMER_INT		INT_TIMER0A
//
///* Variables */
//extern volatile unsigned long g_ulTimeStamp;
//short *g_psDacOutput; /* Buffer that is currently being played */
//short g_psDacBuffer1[DAC_BUFFER_SIZE];
//short g_psDacBuffer2[DAC_BUFFER_SIZE];
//unsigned short g_usDacIndex = 0; /* Index in array */
//unsigned char g_ucDacPlaying = 0; /* Whether audio is currently playing */
//unsigned char g_ucDacQueued = 0; /* Whether a buffer has been queued for play */
//
//unsigned short pusSineTable[] = {32767, 33910, 35052, 36192, 37327, 38457, 39579, 40694, 41799, 42892, 43974, 45042, 46094, 47131, 48150, 49151, 50131, 51090, 52027, 52940, 53829, 54693, 55529, 56338, 57118, 57868, 58588, 59276, 59932, 60555, 61144, 61699, 62218, 62702, 63148, 63558, 63931, 64265, 64561, 64818, 65037, 65216, 65355, 65455, 65515, 65535, 65515, 65455, 65355, 65216, 65037, 64818, 64561, 64265, 63931, 63558, 63148, 62702, 62218, 61699, 61144, 60555, 59932, 59276, 58588, 57868, 57118, 56338, 55529, 54693, 53829, 52940, 52027, 51090, 50131, 49151, 48150, 47131, 46094, 45042, 43974, 42892, 41799, 40694, 39579, 38457, 37327, 36192, 35052, 33910, 32767, 31623, 30481, 29341, 28206, 27076, 25954, 24839, 23734, 22641, 21559, 20491, 19439, 18402, 17383, 16383, 15402, 14443, 13506, 12593, 11704, 10840, 10004, 9195, 8415, 7665, 6945, 6257, 5601, 4978, 4389, 3834, 3315, 2831, 2385, 1975, 1602, 1268, 972, 715, 496, 317, 178, 78, 18, 0, 18, 78, 178, 317, 496, 715, 972, 1268, 1602, 1975, 2385, 2831, 3315, 3834, 4389, 4978, 5601, 6257, 6945, 7665, 8415, 9195, 10004, 10840, 11704, 12593, 13506, 14443, 15402, 16383, 17383, 18402, 19439, 20491, 21559, 22641, 23734, 24839, 25954, 27076, 28206, 29341, 30481, 31623};
//unsigned char ucSineInc = 0;
//
///*
// * Initialises dac timer for use with the DAC
// */
//void init_dac_timer(void) {
//	/* Enable timer peripheral */
//	ROM_SysCtlPeripheralEnable(DAC_TIMER);
//	/* Configure timer to be a periodic 32-bit timer, 44.1kHz */
//	ROM_TimerConfigure(DAC_TIMER_BASE, TIMER_CFG_PERIODIC);
//	ROM_TimerLoadSet(DAC_TIMER_BASE, TIMER_A, ROM_SysCtlClockGet()/44100);
//	/* Set up the interrupts */
//	ROM_IntEnable(DAC_TIMER_INT);
//	ROM_TimerIntEnable(DAC_TIMER_BASE, TIMER_TIMA_TIMEOUT);
//	/* Enable the timer */
//	ROM_TimerEnable(DAC_TIMER_BASE, TIMER_A);
//}
//
///*
// * Initialises the DAC for use
// * DAC is TI-DAC8411, 16-bit SPI-based
// *
// * Configures GPIO pins and SSI0
// */
//void init_dac(void) {
//	g_psDacOutput = g_psDacBuffer1;
//	/* Initialise the SPI interface */
//	ROM_SysCtlPeripheralEnable(SSI_DAC);
//	ROM_SysCtlPeripheralEnable(GPIO_DAC);
//	/* Configure SPI pins - do not need MISO pin for DAC */
//	ROM_GPIOPinConfigure(SSI_TX_DAC);
//	ROM_GPIOPinConfigure(SSI_CLK_DAC);
//	ROM_GPIOPinConfigure(SSI_FSS_DAC);
//	ROM_GPIOPinTypeSSI(GPIO_DAC_BASE, GPIO_DAC_CLK | GPIO_DAC_TX | GPIO_DAC_FSS);
//	/* Configure SSI to use system clock, Mode 1 (Clock polarity 0, data captured on falling edge), 25MHz, 12-bit (24-bit packets) */
//	ROM_SSIConfigSetExpClk(SSI_DAC_BASE, ROM_SysCtlClockGet(), SSI_FRF_MOTO_MODE_1, SSI_MODE_MASTER, 25000000, 12);
//	/* Enable SSI */
//	ROM_SSIEnable(SSI_DAC_BASE);
//	/* Initialise the timer */
//	init_dac_timer();
//}
//
///*
// * Outputs 16-bit data to the DAC
// */
//void dac_output(unsigned short usAudioData) {
//	/* Format the data to DAC compatible - 2 0's, 16 data bits, 6 0's */
//	unsigned long ulUpperHalf = 0, ulBottomHalf = 0;
//	ulUpperHalf = (usAudioData >> 6) & 0x0FFF;
//	ulBottomHalf = (usAudioData << 6) & 0x0FFF;
//	/* Output upper and bottom 12 bits */
//	ROM_SSIDataPut(SSI_DAC_BASE, ulUpperHalf);
//	ROM_SSIDataPut(SSI_DAC_BASE, ulBottomHalf);
//}
//
///*
// * Interrupt handler for the DAC timer
// */
//void timer0_int_handler(void) {
//	/* Need to clear the interrupt flag */
//	ROM_TimerIntClear(DAC_TIMER_BASE, TIMER_TIMA_TIMEOUT);
//	/* If data is queued, play next sample - stop if reach end of data */
//	if(g_ucDacPlaying) {
//		long lAudioData = (long)(g_psDacOutput[g_usDacIndex++]);
//		lAudioData += (1 << 15);
//		dac_output((unsigned short)lAudioData);
//		if(g_usDacIndex >= DAC_BUFFER_SIZE) {
//			next_buffer();
//		}
//	} else {
//		dac_output(0);
//	}
////	dac_output(pusSineTable[ucSineInc++]);
////	if(ucSineInc >= 180) ucSineInc = 0;
//}
//
///*
// * Queues a buffer of DAC_BUFFER_SIZE u_int16t's for play at 44.1kHz
// *
// * Returns true if added successfully, false if there is already a buffer queued
// * Buffer passed to function is copied by value, and can be discarded after
// */
//tBoolean play_buffer(short *psBuffer) {
//	unsigned short i;
//	/* Check whether a buffer has already been queued */
//	if(g_ucDacQueued) {
//		return false;
//	}
//	/* Copy data into queued buffer */
//	if(g_psDacOutput == g_psDacBuffer1) {
//		for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
//			g_psDacBuffer2[i] = psBuffer[i];
//		}
//	} else {
//		for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
//			g_psDacBuffer1[i] = psBuffer[i];
//		}
//	}
//	g_ucDacQueued = 1;
//	/* Check whether the DAC is playing data, if not load the buffer now */
//	if(!g_ucDacPlaying) {
//		g_ucDacPlaying = 1;
//		next_buffer();
//	}
//	return true;
//}
//
///*
// * Loads the next block into the buffer to be played
// *
// * Called by Timer0 interrupt when previous buffer ends
// */
//void next_buffer(void) {
//	/* Check that we have a buffer queued */
//	if(!g_ucDacQueued) {
//		g_ucDacPlaying = 0;
//		unsigned short i;
//		for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
//			g_psDacBuffer1[i] = -(1 << 15);
//			g_psDacBuffer2[i] = -(1 << 15);
//		}
//		return;
//	}
//	/* Change the queued buffer for the playing buffer */
//	if(g_psDacOutput == g_psDacBuffer1) {
//		g_psDacOutput = g_psDacBuffer2;
//	} else {
//		g_psDacOutput = g_psDacBuffer1;
//	}
//	/* We no longer have data queued */
//	g_ucDacQueued = 0;
//	g_usDacIndex = 0;
//}
