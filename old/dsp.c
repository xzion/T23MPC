/*
 * dsp.c
 *
 * contains functionality to perform DSP on samples
 */

/* Library headers */
#include "arm_math.h"
#include "inc/hw_ints.h"
#include "inc/hw_ssi.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "third_party/fatfs/src/ff.h"

/* My file headers */
#include "dac.h"
#include "samples.h"
#include "dsp.h"
#include "sdc.h"
#include "uart.h"

/* Variables */
short g_psFXBuffer[DSP_BUFFER_SIZE];
unsigned short g_usFXIndex = 0;

/*
 * Applies FX to audio buffer
 *
 * Each FX should have at least 128 levels (per parameter)
 */
void process_dsp(short *psBuffer, unsigned long *pulADCData) {
	FXType xFX = g_sConfig.xFX1;

	/* If delay effect is active, store the current input */
	if(g_sConfig.xFX1 == DL || g_sConfig.xFX2 == DL) {
		unsigned short i;
		for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
			g_psFXBuffer[g_usFXIndex++] = psBuffer[i];
			g_usFXIndex %= DSP_BUFFER_SIZE;
		}
	}

	unsigned char i = 0;
	/* Perform DSP on samples for FX1 then FX2 */
//	for(i = 0; i < 2; ++i) {
		switch(xFX) {
		case DL:
			FX_delay(pulADCData[i*2], pulADCData[i*2 + 1] << 2, psBuffer);
			break;
		case EC:
			FX_echo(pulADCData[i*2], pulADCData[i*2 + 1] << 2, psBuffer);
			break;
		case DC:
			FX_decimator(pulADCData[i*2] >> 5, pulADCData[i*2 + 1] >> 5, psBuffer);
			break;
		case KO:
			FX_ko(pulADCData[i*2] >> 5, pulADCData[i*2 + 1] >> 5, psBuffer);
			break;
		default:
			FX_filter(xFX, pulADCData[i*2], pulADCData[i*2 + 1], psBuffer);
			break;
		}
		xFX = g_sConfig.xFX2;
//	}

	/* If echo effect is active, store the current output */
	if(g_sConfig.xFX1 == EC || g_sConfig.xFX2 == EC) {
		unsigned short i;
		for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
			g_psFXBuffer[g_usFXIndex++] = psBuffer[i];
			g_usFXIndex %= DSP_BUFFER_SIZE;
		}
	}
}

/*
 * Applies the delay effect to audio data
 *
 * Reads old input data from SD card and adds it on to audio buffer
 */
void FX_delay(unsigned long ulGain, unsigned short usOffset, short *psBuffer) {
	/* Get the old input data based on usOffset */
	unsigned short usIndex = g_usFXIndex - usOffset - 512;
	usIndex %= DSP_BUFFER_SIZE;
	if(usIndex == 0) return;
	/* Add old input data on to audio buffer */
	unsigned short i;
	for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
		psBuffer[i] += g_psFXBuffer[usIndex++] * ulGain >> 12;
		usIndex %= DSP_BUFFER_SIZE;
	}
}

/*
 * Applies the echo effect to audio data
 *
 * Reads old output data from SD card and adds it on to audio buffer
 */
void FX_echo(unsigned long ulGain, unsigned short usOffset, short *psBuffer) {
	/* Get the old output data based on usOffset */
	unsigned short usIndex = g_usFXIndex - usOffset - 512;
	usIndex %= DSP_BUFFER_SIZE;
	if(usIndex == 0) return;
	/* Add old output data on to audio buffer */
	unsigned short i;
	for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
		psBuffer[i] += g_psFXBuffer[usIndex++] * (ulGain >> 1) >> 12;
		usIndex %= DSP_BUFFER_SIZE;
	}
}

/*
 * Applies the decimator & bitcrusher effects to audio data
 *
 * Decimator down-samples the data by and then uses linear interpolation to bring it back to 44.1kHz
 * Bitcrusher reduces the bit-depth (i.e. introduces quantisation distortion)
 */
void FX_decimator(unsigned char ucDecimation, unsigned char ucBitcrush, short *psBuffer) {
	/* Decimate the audio data - pick every mth sample to keep */
	if(ucDecimation > 1) {
		unsigned short n = 0, m, i;
		for(m = ucDecimation; m < DAC_BUFFER_SIZE; m += ucDecimation) {
			/* Now replace every sample in between with a linear interpolation */
			unsigned char ucMValue = psBuffer[m], ucNValue = psBuffer[n];
			for(i = n; i < m; ++i) {
				psBuffer[i] = (i - n) * (ucMValue - ucNValue)/(m - n) + ucNValue;
			}
			n = m;
		}
	}
	/* Bitcrush the audio data - integer divide and then multiply */
	if(ucBitcrush > 1) {
		unsigned short i;
		for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
			psBuffer[i] /= (ucBitcrush << 4);
			psBuffer[i] *= (ucBitcrush << 4);
		}
	}
}

/*
 * Applies the bitwise KO to audio data
 *
 * 2 dials set a number between 0 and 2^16
 * 1st number is AND'd with audio
 * 2nd number is XOR'd with audio
 */
void FX_ko(unsigned char ucAnd, unsigned char ucXor, short *psBuffer) {
	/* Calculate the numbers */
	unsigned short usAndNo = ucAnd << 9;
	unsigned short usXorNo = ucXor << 9;
	/* knock-out the audio */
	unsigned short i;
	for(i = 0; i < DAC_BUFFER_SIZE; ++i) {
		psBuffer[i] = (psBuffer[i] & ~usAndNo) ^ usXorNo;
	}
}

/*
 * Applies a given filter to the audio data in the given buffer
 *
 * xFilter can be == LP, HP, BP or BS (enum type, see samples.h)
 * ulQ is the Q-factor/resonance of the filter as a unit-less number from 0-4095
 * ulFc is the cut-off frequency as a unit-less number from 0-4095
 * psBuffer is a buffer of size DAC_BUFFER_SIZE holding the audio data
 *
 * The ranges of Q and Fc are unspecified, so just do what seems right for you (as long as you have at least 128 levels)
 * At the moment, DAC_BUFFER_SIZE is 512 which equates to ~11.61ms (@44.1kHz). If this is too small a window to filter, let me know
 */
void FX_filter(FXType xFilter, unsigned long ulQ, unsigned long ulFc, short *psBuffer) {
	/* Array for the filter coefficients
	 * 					 					b2*z^-2 + b1*z^-1 + b0
	 * the transfer equation has the form  ----------------------
	 * 									    a2*z^-2 + a1*z^-1 + 1
	 * (a1 and a2 are constant, regardless of filter type)
	 * The pfFilterCoeffs array is: [b0, b1, b2, a1, a2]
	 */
	float32_t pfFilterCoeffs[5];
	float32_t pfStateArray[4];
	arm_biquad_casd_df1_inst_f32 S;

	/* The buffers required for arm_biquad_cascade_df1_f32 must be float32_t */
	float32_t pfBuffer[DAC_BUFFER_SIZE >> 4];
	float32_t pfOutBuffer[DAC_BUFFER_SIZE >> 4];

	/* Adjust ulFc and Q to be (roughly) between 0 and 20 kHz and 0.01 and 100, respectively */
	ulFc = 5 * ulFc;
	float32_t Q = ulQ * 0.0025 + 0.1;

	/* fWWarped = tan(3.14 * Fc / Fsampling) */
	float32_t fWWarped = arm_sin_f32(3.14 * (float32_t)ulFc / 44100) / arm_cos_f32(3.14 * (float32_t)ulFc / 44100);

	/* Every filter has slightly different coefficients that have been derived
	 * from the transfer functions of second order filters */
	pfFilterCoeffs[3] = 2 * Q * (pow(fWWarped, 2) - 1) / (pow(fWWarped, 2) + Q + Q * pow(fWWarped, 2));
	pfFilterCoeffs[4] = 1 - 2 * fWWarped / (fWWarped + Q + Q * pow(fWWarped, 2));
	switch(xFilter) {
	case LP:
		pfFilterCoeffs[0] = Q * pow(fWWarped, 2) / (fWWarped + Q + Q * pow(fWWarped, 2));
		pfFilterCoeffs[1] = 2 * Q * pow(fWWarped, 2) / (fWWarped + Q + Q * pow(fWWarped, 2));
		pfFilterCoeffs[2] = pfFilterCoeffs[0];
		break;
	case HP:
		pfFilterCoeffs[0] = Q / (fWWarped + Q + Q * pow(fWWarped, 2));
		pfFilterCoeffs[1] = -2 * Q / (fWWarped + Q + Q * pow(fWWarped, 2));
		pfFilterCoeffs[2] = pfFilterCoeffs[0];
		break;
	case BP:
		pfFilterCoeffs[0] = fWWarped / (fWWarped + Q + Q * pow(fWWarped, 2));
		pfFilterCoeffs[1] = 0;
		pfFilterCoeffs[2] = -pfFilterCoeffs[0];
		break;
	case BS:
		pfFilterCoeffs[0] = Q * (pow(fWWarped, 2) + 1) / (fWWarped + Q + Q * pow(fWWarped, 2));
		pfFilterCoeffs[1] = 2 * Q * (pow(fWWarped, 2) - 1) / (fWWarped + Q + Q * pow(fWWarped, 2));
		pfFilterCoeffs[2] = pfFilterCoeffs[0];
		break;
	}

	/* Initiate the filter and run data through */
	arm_biquad_cascade_df1_init_f32(&S, 1, pfFilterCoeffs, pfStateArray);
	unsigned char i;
	for(i = 0; i < 15; ++i) {
		unsigned char j;
		/* Convert the short psBuffer to float */
		unsigned long ulStart = i*(DAC_BUFFER_SIZE >> 4);
		for (j = 0; j < DAC_BUFFER_SIZE >> 4; ++j) {
			pfBuffer[j] = (float32_t)psBuffer[j + ulStart];
		}
		/* Filter the data */
		arm_biquad_cascade_df1_f32(&S, pfBuffer, pfOutBuffer, DAC_BUFFER_SIZE >> 4);
		/* Convert the data back to unsigned short and overwrite the buffer */
		for (j = 0; j < DAC_BUFFER_SIZE >> 4; ++j) {
			psBuffer[j + ulStart] = (short)pfOutBuffer[j];
		}
	}
}
