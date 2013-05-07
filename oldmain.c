///*
// * main.c
// *
// * contains main function
// *
// *
// * To-do List
// * 		TODO Test sample playing
// * 		TODO Audio should keep playing if echo/delay is still going - leave this until after I've tested sample playing
// *
// */
//
///* Library headers */
//#include "inc/hw_ints.h"
//#include "inc/hw_types.h"
//#include "driverlib/rom.h"
//#include "driverlib/sysctl.h"
//#include "third_party/fatfs/src/diskio.h"
//
///* My file headers */
//#include "old/buttons.h"
//#include "old/dac.h"
//#include "old/uart.h"
//#include "old/dials.h"
//#include "old/sdc.h"
//#include "old/samples.h"
//#include "old/dsp.h"
//
///* Variables */
//volatile unsigned long g_ulTimeStamp = 0; /* Time since startup in units of 10ms */
//extern unsigned long g_ulIdleTimeout;
//
///*
// * Handler for SysTick interrupt
// *
// * FatFS requires a 10ms tick for timing
// */
//void SysTickHandler(void) {
//	/* Let FatFS know */
//    disk_timerproc();
//    /* Increment time-stamp */
//    ++g_ulTimeStamp;
//    if(g_ulIdleTimeout > 0) {
//    	g_ulIdleTimeout--;
//    }
//}
//
///* Main function
// *
// * Initialise everything
// * Not yet sure what functionality will be, should mostly be interrupt-driven
// */
//void main(void) {
//	/* Set clock to 80MHz */
//	ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN );
//
//	/* Enable SysTick to provide a 10ms tick for FatFS and timestamp */
//	ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 100);
//	ROM_SysTickEnable();
//	ROM_SysTickIntEnable();
//
//	/* Enable interrupts */
//	ROM_IntMasterEnable();
//
//	/* Enable the FPU - won't be performing FP operations in interrupts */
//	ROM_FPUEnable();
//	ROM_FPUStackingDisable();
//
//	/* Call initialisation functions (buttons must come first as this enables GPIOs) */
//	init_buttons();
//	init_adc();
//	init_dac();
//#if DEBUG
//	init_uart();
//#endif
//	init_sdc();
//	init_samples();
//
//	/* Program variables */
//	unsigned char j = 0, pucButtons[16], pucLeds[16];
//	unsigned long pulDebounce[16], i;
//	for(i = 0; i < 16; ++i)	pulDebounce[i] = pucButtons[i] = pucLeds[i] = 0;
//	unsigned long pulADCData[4];
//	for(i = 0; i < 4; ++i) pulADCData[i] = 0;
//	for(i = 0; i < DSP_BUFFER_SIZE; ++i) g_psFXBuffer[i] = 0;
//
//	/* Program loop - runs every 10 ms */
//	for(;;) {
//#if DEBUG
////		unsigned char k = 0;
////		for(k = 0; k < 16; ++k) pucLeds[k] = pucButtons[k];
//		if(g_ulTimeStamp % 100 == 0) {
//			UARTprintf("\n%d\n", g_ulTimeStamp / 100);
//			UARTprintf("FX1 %4dmV\t%4dmV\n", (pulADCData[0]*3300)>>12, (pulADCData[1]*3300)>>12);
//			UARTprintf("FX2 %4dmV\t%4dmV\n", (pulADCData[2]*3300)>>12, (pulADCData[3]*3300)>>12);
//			UARTprintf("Buttons pressed: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", pucButtons[0], pucButtons[1], pucButtons[2], pucButtons[3], pucButtons[4], pucButtons[5], pucButtons[6], pucButtons[7], pucButtons[8], pucButtons[9], pucButtons[10], pucButtons[11], pucButtons[12], pucButtons[13], pucButtons[14], pucButtons[15]);
//			UARTprintf("Sample 1 = %d\tSample 2 = %d\n", g_sConfig.ucSample1Index, g_sConfig.ucSample2Index);
//		}
//#endif
//		unsigned long ulTimeStart = g_ulTimeStamp;
//		/* Refresh ADC data */
//		adc_data_get(pulADCData);
//		adc_trigger();
//		/* Read button matrix */
//		scan_buttons(pucButtons, pulDebounce);
//		if(i = process_buttons(pucButtons, pucLeds)) {
//			/* Configuration upload/download - wait for it to finish and reset/keep going */
//			while(g_ulTimeStamp - ulTimeStart < 100) {
//				set_leds(pucLeds, j++);
//				unsigned short k = 500;
//				while(k--);
//			}
//			pucButtons[1] = pucButtons[2] = 0;
//			while(1) {
//				scan_buttons(pucButtons, pulDebounce);
//				if(process_buttons(pucButtons, pucLeds) == i) {
//					break;
//				}
//				pucLeds[i + 1] = 2;
//				set_leds(pucLeds, j++);
//				unsigned short k = 500;
//				while(k--);
//			}
//			pucButtons[1] = pucButtons[2] = 0;
//			ulTimeStart = g_ulTimeStamp;
//			while(g_ulTimeStamp - ulTimeStart < 100) {
//				set_leds(pucLeds, j++);
//				unsigned short k = 500;
//				while(k--);
//			}
//			while(!sdc_available());
//			if(i == 1) {
//				init_samples();
//				continue;
//			}
//		}
//
//		/* Get next block of audio ready */
//		process_samples(pulADCData, pucLeds);
//		/* Wait until 10ms is up */
//		while(ulTimeStart == g_ulTimeStamp) {
//			set_leds(pucLeds, j++);
//			unsigned short k = 500;
//			while(k--);
//		}
//	}
//}
