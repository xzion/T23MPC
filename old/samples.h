///*
// * samples.h
// *
// * contains function prototypes for samples.c
// */
//
//#ifndef SAMPLES_H_
//#define SAMPLES_H_
//
//typedef enum {
//	LP, /* Low-pass */
//	HP, /* High-pass */
//	BP, /* Band-pass */
//	BS, /* Band-stop/Notch */
//	DL, /* Delay */
//	EC, /* Echo */
//	DC, /* Decimator/Bitcrusher */
//	KO, /* Bitwise KO */
//} FXType;
//
//typedef struct {
//	FIL xFile1; /* File structures for samples */
//	FIL xFile2; /* File structures for samples */
//	unsigned char ucSample1Index;
//	unsigned char ucSample2Index;
//	unsigned char pucLoopIntervals[16];
//	FXType xFX1; /* Type of FX1 effect */
//	FXType xFX2; /* Type of FX2 effect */
//	unsigned char ucTempo; /* Tempo in bpm */
//	unsigned short usHold; /* Whether each button is Hold (1) or Latch (0) */
//	tBoolean bDefault; /* True if no configuration - default behaviour */
//} ConfigState;
//
//extern ConfigState g_sConfig;
//
//extern void init_samples(void);
//extern void stop_playing(unsigned char ucIndex);
//extern void start_playing(unsigned char ucIndex);
//extern void process_samples(unsigned long* pulADCData, unsigned char *pucLeds);
//
//#endif /* SAMPLES_H_ */
