///*
// * dials.c
// *
// *  contains functions to interface with ADC for reading dials
// */
//
///* Library headers */
//#include "inc/hw_ints.h"
//#include "inc/hw_memmap.h"
//#include "inc/hw_types.h"
//#include "driverlib/adc.h"
//#include "driverlib/debug.h"
//#include "driverlib/gpio.h"
//#include "driverlib/pin_map.h"
//#include "driverlib/rom.h"
//#include "driverlib/sysctl.h"
//
///* My file headers */
//#include "dials.h"
//
///* Definitions */
//#define ADC_PIN_FX1G		GPIO_PIN_3
//#define ADC_PIN_FX1F		GPIO_PIN_2
//#define ADC_PIN_FX2G		GPIO_PIN_5
//#define ADC_PIN_FX2F		GPIO_PIN_4
//#define ADC_GPIO_BASE		GPIO_PORTE_BASE
//#define ADC_CH_FX1G			ADC_CTL_CH0
//#define ADC_CH_FX1F			ADC_CTL_CH1
//#define ADC_CH_FX2G			ADC_CTL_CH8
//#define ADC_CH_FX2F			ADC_CTL_CH9
//
///* Variables */
//extern volatile unsigned long g_ulTimeStamp;
//
///*
// * initialises ADC module for use
// *
// * Enables peripherals, GPIO pins, timers etc.
// */
//void init_adc(void) {
//	/* Enable ADC peripheral */
//	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
//	/* Configure the GPIO pins */
//	ROM_GPIOPinTypeADC(ADC_GPIO_BASE, ADC_PIN_FX1G | ADC_PIN_FX1F | ADC_PIN_FX2G | ADC_PIN_FX2F);
//	/* Set ADC to sequence 1 (perform up to 4 conversions) and manual trigger */
//	ROM_ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_PROCESSOR, 0);
//	/* Set sequence steps (each sequence step corresponds to reading a different dial) */
//	ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CH_FX1G);
//	ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CH_FX1F);
//	ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CH_FX2G);
//	ROM_ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CH_FX2F | ADC_CTL_END);
//	/* Enable sequence */
//	ROM_ADCSequenceEnable(ADC0_BASE, 1);
//}
//
///*
// * triggers ADC conversion
// */
//inline void adc_trigger(void) {
//	ROM_ADCProcessorTrigger(ADC0_BASE, 1);
//}
//
///*
// * retrieves samples from ADC
// *
// * returns true if samples ready
// * puts data in the given array (4*ul), does not allocate memory
// * 		0: FX1 Gain
// * 		1: FX1 Cut-off
// * 		2: FX2 Gain
// * 		3: FX2 Cut-off
// */
//long adc_data_get(unsigned long* pulData) {
//	/* Fetch conversion result from each input */
//	return ROM_ADCSequenceDataGet(ADC0_BASE, 1, pulData);
//}
