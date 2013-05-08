///*
// * samples.c
// *
// * functionality for handling play-back of samples
// */
//
///* Library headers */
//#include "inc/hw_ints.h"
//#include "inc/hw_ssi.h"
//#include "inc/hw_types.h"
//#include "inc/hw_memmap.h"
//#include "driverlib/debug.h"
//#include "driverlib/gpio.h"
//#include "driverlib/interrupt.h"
//#include "driverlib/pin_map.h"
//#include "driverlib/rom.h"
//#include "driverlib/sysctl.h"
//#include "driverlib/timer.h"
//#include <string.h>
//#include "third_party/fatfs/src/ff.h"
//
///* My file headers */
//#include "buttons.h"
//#include "dac.h"
//#include "sdc.h"
//#include "samples.h"
//#include "dsp.h"
//#include "uart.h"
//
///* Definitions */
//#define TEMPO_TIMER			SYSCTL_PERIPH_TIMER1
//#define TEMPO_TIMER_BASE	TIMER1_BASE
//#define TEMPO_TIMER_INT		INT_TIMER1A
//#define BUTTON_TEMPO_LED	GPIO_PIN_6
//
///* Variables */
//ConfigState g_sConfig;
//unsigned long g_ulTempoCount = 0;
//
///*
// * Initialises the sample module
// *
// * Will load configuration from SD card
// * If no SD card is available, will default to playing tones
// * Waits until PC is not accessing card
// */
//void init_samples(void) {
//	FIL xConfig;
//	g_sConfig.bDefault = false;
//	g_sConfig.ucSample1Index = 0xFF;
//	g_sConfig.ucSample2Index = 0xFF;
//	/* Check there is a configuration file on the SD card */
//	if(!sdc_open(&xConfig, 0xFF)) {
//		g_sConfig.bDefault = true;
//		g_sConfig.ucTempo = 120;
//	}
//	/* Read the configuration file into a buffer */
//	char pcBuffer[64];
//	if(!sdc_read(&xConfig, pcBuffer, 64)) {
//		/* Couldn't read SD card - default behaviour */
//		g_sConfig.bDefault = true;
//		g_sConfig.ucTempo = 120;
//	}
//	/* Parse buffer and set configuration */
//	unsigned char i = 0, j = 0, ucLine = 0, ucIndex = 0;
//	if(!g_sConfig.bDefault) {
//		for(i = 0; (i < 64) && pcBuffer[i]; ++i) {
//			/* Split using \n delimiters */
//			if(pcBuffer[i] == '\n') {
//				++ucLine;
//				/* Loop intervals */
//				if(ucLine == 1) {
//					unsigned char k;
//					for(k = 0; k < 16; ++k) {
//						g_sConfig.pucLoopIntervals[k] = (pcBuffer[2*k] - '0')*10 + pcBuffer[1 + 2*k] - '0';
//					}
//				}
//				/* FX1/FX2 - determine filter type */
//				else if(ucLine == 2 || ucLine == 3) {
//					FXType xFX;
//					if(strstr(pcBuffer + ucIndex, "LP") == pcBuffer + ucIndex) {
//						xFX = LP;
//					} else if(strstr(pcBuffer + ucIndex, "HP") == pcBuffer + ucIndex) {
//						xFX = HP;
//					} else if(strstr(pcBuffer + ucIndex, "BP") == pcBuffer + ucIndex) {
//						xFX = BP;
//					} else if(strstr(pcBuffer + ucIndex, "BS") == pcBuffer + ucIndex) {
//						xFX = BS;
//					} else if(strstr(pcBuffer + ucIndex, "DL") == pcBuffer + ucIndex) {
//						xFX = DL;
//					} else if(strstr(pcBuffer + ucIndex, "EC") == pcBuffer + ucIndex) {
//						xFX = EC;
//					} else if(strstr(pcBuffer + ucIndex, "DC") == pcBuffer + ucIndex) {
//						xFX = DC;
//					} else if(strstr(pcBuffer + ucIndex, "KO") == pcBuffer + ucIndex) {
//						xFX = KO;
//					} else {
//						xFX = LP;
//					}
//					if(ucLine == 2) g_sConfig.xFX1 = xFX;
//					else g_sConfig.xFX2 = xFX;
//				}
//				/* Tempo */
//				else if(ucLine == 4) {
//					g_sConfig.ucTempo = 0;
//					for(j = ucIndex; j < i - 1; ++j) {
//						g_sConfig.ucTempo = g_sConfig.ucTempo * 10 + pcBuffer[j] - '0';
//					}
//				}
//				/* Button settings */
//				else if(ucLine == 5) {
//					g_sConfig.usHold = 0;
//					for(j = ucIndex; (pcBuffer[j] >= '0') && (pcBuffer[j] <= '9'); ++j) {
//						g_sConfig.usHold = g_sConfig.usHold * 16 + pcBuffer[j] - '0';
//					}
//					break;
//				}
//				ucIndex = i + 1;
//			}
//		}
//	}
//	/* Start tempo LED going - periodic 32-bit timer */
//	ROM_SysCtlPeripheralEnable(TEMPO_TIMER);
//	ROM_TimerConfigure(TEMPO_TIMER_BASE, TIMER_CFG_PERIODIC);
//	ROM_TimerLoadSet(TEMPO_TIMER_BASE, TIMER_A, (SysCtlClockGet() / (g_sConfig.ucTempo * 32)) * 60);
//	/* Set up the interrupts */
//	ROM_IntEnable(TEMPO_TIMER_INT);
//	ROM_TimerIntEnable(TEMPO_TIMER_BASE, TIMER_TIMA_TIMEOUT);
//	/* Enable the timer */
//	ROM_TimerEnable(TEMPO_TIMER_BASE, TIMER_A);
//
//#if DEBUG
//	UARTprintf("Configuration:\n\tLoops: %d%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d\n", g_sConfig.pucLoopIntervals[0], g_sConfig.pucLoopIntervals[1], g_sConfig.pucLoopIntervals[2], g_sConfig.pucLoopIntervals[3], g_sConfig.pucLoopIntervals[4], g_sConfig.pucLoopIntervals[5], g_sConfig.pucLoopIntervals[6], g_sConfig.pucLoopIntervals[7], g_sConfig.pucLoopIntervals[8], g_sConfig.pucLoopIntervals[9], g_sConfig.pucLoopIntervals[10], g_sConfig.pucLoopIntervals[11], g_sConfig.pucLoopIntervals[12], g_sConfig.pucLoopIntervals[13], g_sConfig.pucLoopIntervals[14], g_sConfig.pucLoopIntervals[15]);
//	UARTprintf("\tFX1: %d\tFX2: %d\n", g_sConfig.xFX1, g_sConfig.xFX2);
//	UARTprintf("\tTempo: %d\n", g_sConfig.ucTempo);
//	UARTprintf("\tHold/Latch: %d\n\n", g_sConfig.usHold);
//#endif
//}
//
///*
// * Interrupt handler for the tempo timer
// */
//void timer1_int_handler(void) {
//	/* Clear the interrupt flag */
//	ROM_TimerIntClear(TEMPO_TIMER_BASE, TIMER_TIMA_TIMEOUT);
//
//	++g_ulTempoCount;
//	/* Blink tempo LED on the beat */
//	if(g_ulTempoCount % 32 <= 15) {
//		ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_TEMPO_LED, BUTTON_TEMPO_LED);
//	} else {
//		ROM_GPIOPinWrite(GPIO_PORTB_BASE, BUTTON_TEMPO_LED, 0);
//	}
//}
//
///*
// * Stops playing a given sample index
// */
//void stop_playing(unsigned char ucIndex) {
//	if(g_sConfig.ucSample1Index != ucIndex && g_sConfig.ucSample2Index != ucIndex) return;
//	/* Update program state */
//	if(g_sConfig.ucSample2Index == ucIndex) {
//		g_sConfig.ucSample2Index = 0xFF;
//		sdc_close(&g_sConfig.xFile2);
//	} else if(g_sConfig.ucSample1Index == ucIndex) {
//		g_sConfig.ucSample1Index = g_sConfig.ucSample2Index;
//		g_sConfig.ucSample2Index = 0xFF;
//		unsigned long ulTemp = f_tell(&g_sConfig.xFile2);
//		sdc_close(&g_sConfig.xFile1);
//		sdc_open(&g_sConfig.xFile1, g_sConfig.ucSample1Index);
//		f_lseek(&g_sConfig.xFile1, ulTemp);
//	}
//	/* Stop looping if doing so */
//	g_usLooping &= ~(1 << ucIndex);
//}
//
///*
// * Starts playing a given sample index
// */
//void start_playing(unsigned char ucIndex) {
//	if(ucIndex == g_sConfig.ucSample1Index || ucIndex == g_sConfig.ucSample2Index) return;
//	/* Update program state */
//	if(g_sConfig.ucSample1Index == 0xFF) {
//		/* No samples playing */
//		g_sConfig.ucSample1Index = ucIndex;
//		sdc_open(&g_sConfig.xFile1, ucIndex);
//	} else if(g_sConfig.ucSample2Index == 0xFF) {
//		/* 1 sample playing */
//		g_sConfig.ucSample2Index = ucIndex;
//		sdc_open(&g_sConfig.xFile2, ucIndex);
//	} else {
//		/* 2 samples playing */
//		stop_playing(g_sConfig.ucSample1Index);
//		g_sConfig.ucSample2Index = ucIndex;
//		sdc_open(&g_sConfig.xFile2, ucIndex);
//	}
//}
//
///*
// * Fetches next block of audio for playing and adds it to DAC queue
// *
// * Will read in data from SD card, pass to the DSP for filtering, then pass to the DAC for play
// * Which samples are currently playing is determined by g_sConfig
// */
//void process_samples(unsigned long* pulADCData, unsigned char *pucLeds) {
//	short psBuffer[DAC_BUFFER_SIZE], i;
//	for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
//		psBuffer[i] = -32768;
//	}
//	if(g_sConfig.bDefault || g_sConfig.ucSample1Index == 0xFF) {
//		/* Default behaviour (no config file)*/
//	} else {
//		/* Fetch next block of data for each sample */
//		while(!sdc_available());
//		if(!sdc_read(&g_sConfig.xFile1, psBuffer, DAC_BUFFER_SIZE * 2)) return;
//		/* If we've reached EOF, reset the pointer and stop playing if latch and not looping */
//		if(f_eof(&g_sConfig.xFile1)) {
//			/* If we're looping, only start again if on beat */
//			if(g_usLooping & (1 << g_sConfig.ucSample1Index)) {
//				if(g_ulTempoCount % (32 / g_sConfig.pucLoopIntervals[g_sConfig.ucSample1Index]) == 0) {
//					if(!sdc_reset(&g_sConfig.xFile1)) return;
//				}
//			} else {
//				if(!sdc_reset(&g_sConfig.xFile1)) return;
//			}
//			/* If latched button and not looping, stop playing now */
//			if(!(g_sConfig.usHold & (1 << g_sConfig.ucSample1Index)) && !(g_usLooping & (1 << g_sConfig.ucSample1Index))) {
//				stop_playing(g_sConfig.ucSample1Index);
//				pucLeds[g_sConfig.ucSample1Index] = 0;
//			}
//		}
//		if(g_sConfig.ucSample2Index != 0xFF) {
//			short psBuffer2[DAC_BUFFER_SIZE];
//			for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
//				psBuffer2[i] = -32768;
//			}
//			while(!sdc_available());
//			if(!sdc_read(&g_sConfig.xFile2, psBuffer2, DAC_BUFFER_SIZE * 2)) return;
//			/* If we've reached EOF, reset the pointer and stop playing if latch and not looping */
//			if(f_eof(&g_sConfig.xFile2)) {
//				/* If we're looping, only start again if on beat */
//				if(g_usLooping & (1 << g_sConfig.ucSample1Index)) {
//					if(g_ulTempoCount % (32 / g_sConfig.pucLoopIntervals[g_sConfig.ucSample2Index]) == 0) {
//						if(!sdc_reset(&g_sConfig.xFile2)) return;
//					}
//				} else {
//					if(!sdc_reset(&g_sConfig.xFile2)) return;
//				}
//				/* If latched button and not looping, stop playing now */
//				if(!(g_sConfig.usHold & (1 << g_sConfig.ucSample2Index)) && !(g_usLooping & (1 << g_sConfig.ucSample2Index))) {
//					stop_playing(g_sConfig.ucSample2Index);
//					pucLeds[g_sConfig.ucSample2Index] = 0;
//				}
//			}
//			/* If we have two samples, need to mix them together */
//			for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
//				psBuffer[i] = (psBuffer[i] >> 1) + (psBuffer2[i] >> 1);
//			}
//		}
//	}
//	/* Perform DSP on samples and output to DAC */
//	process_dsp(psBuffer, pulADCData);
//	while(!play_buffer(psBuffer)) {
//		set_leds(pucLeds, i++);
//		unsigned short k = 500;
//		while(k--);
//	}
//}
