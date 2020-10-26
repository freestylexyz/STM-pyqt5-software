/*
 * DSP_FUNCTIONS.h
 *
 *  Created on: Jun 2, 2016
 *      Author: Ho Group
 *
 */

#ifndef DSP_FUNCTIONS_H_
#define DSP_FUNCTIONS_H_

#include "F28x_Project.h"     // DSP28x Headerfile

#include <stdlib.h>

//##########################GENERAL PARAMETERS###############################
#define FIRMWARE_VERSION "0.8 10172020"
#define NULL_VALUE '%'
#define DELAY_CONVERTION_FACTOR	11.62573
#define DEBOUNCING_DELAY_US	15 //2.5 us rising time + 2.5 us falling time + 10 us reading time
#define BLINK_DELAY_US	500000 //delay used to blink some LEDs in order to test outputs
#define XXMSK1  4
#define XXMSK2  2

//##########################BOARD PINOUT DEFINES#############################
//IN_DATA PINS
#define IN_DATA_BIT_0	32      // -- GpioDataRegs.GPCDAT.bit.GPIO71
#define IN_DATA_BIT_1	19      // -- GpioDataRegs.GPCDAT.bit.GPIO90
#define IN_DATA_BIT_2	18      // -- GpioDataRegs.GPCDAT.bit.GPIO89
#define IN_DATA_BIT_3	67      // -- GpioDataRegs.GPBDAT.bit.GPIO41
#define IN_DATA_BIT_4	60      // -- GpioDataRegs.GPBDAT.bit.GPIO60
#define IN_DATA_BIT_5	22      // -- GpioDataRegs.GPBDAT.bit.GPIO61
#define IN_DATA_BIT_6	65      // -- GpioDataRegs.GPCDAT.bit.GPIO65
#define IN_DATA_BIT_7	41      // -- GpioDataRegs.GPCDAT.bit.GPIO69

//OUT_DATA PINS
#define OUT_DATA_BIT_0  2       // -- GpioDataRegs.GPADAT.bit.GPIO14
#define OUT_DATA_BIT_1  3       // -- GpioDataRegs.GPADAT.bit.GPIO15
#define OUT_DATA_BIT_2  4       // -- GpioDataRegs.GPADAT.bit.GPIO16
#define OUT_DATA_BIT_3  5       // -- GpioDataRegs.GPADAT.bit.GPIO17
#define OUT_DATA_BIT_4  24      // -- GpioDataRegs.GPADAT.bit.GPIO20
#define OUT_DATA_BIT_5  16      // -- GpioDataRegs.GPADAT.bit.GPIO21
#define OUT_DATA_BIT_6  6       // -- GpioDataRegs.GPADAT.bit.GPIO2
#define OUT_DATA_BIT_7  7       // -- GpioDataRegs.GPADAT.bit.GPIO3

//CHANNEL_SELECT PINS
#define CH_0	64      // -- GpioDataRegs.GPCDAT.bit.GPIO64 2 SC10
#define CH_1	63      // -- GpioDataRegs.GPBDAT.bit.GPIO63 6 SC11
#define CH_2	11      // -- GpioDataRegs.GPADAT.bit.GPIO19 5 SC12
#define CH_3	10      // -- GpioDataRegs.GPADAT.bit.GPIO18 1 SCK1
#define CH_4	9       // -- GpioDataRegs.GPADAT.bit.GPIO11 4 SRD1
#define CH_5	8       // -- GpioDataRegs.GPADAT.bit.GPIO10 3 STD1

//20 BIT DAC PINS
#define HIGH_RES_DAC_0 61  // -- 1 SCK1
#define HIGH_RES_DAC_1 123 // -- 2 SC10
#define HIGH_RES_DAC_2 122 // -- 3 STD1
#define HIGH_RES_DAC_3 58  // -- 4 SRD1
#define HIGH_RES_DAC_4 59  // -- 5 SC12
#define HIGH_RES_DAC_5 124 // -- 6 SC11

//COMMON VOLTAGES

#define INPUT_ZERO 0b1000000000000000
#define INPUT_MAX 0b1111111111111111
#define OUTPUT_ZERO 0b0000100000000000
#define OUTPUT_MAX 0b0000111111111111

//CHANNELS
#define NUM_OUT_CHANNEL 16
#define BIAS_OUT_CHANNEL 0b00000000 //0
#define ZOFF_OUT_CHANNEL 0b00010000 //1
#define X_OUT_CHANNEL 0b00100000 //2
#define Y_OUT_CHANNEL 0b00110000 //3
#define Z_OUTER_CHANNEL 0b01000000 //4
#define I_OUT_CHANNEL 0b10000000 //8
#define DITHER_OUT_CHANNEL 0b11010000 //13
#define FB_OUT_CHANNEL 0b11100000 //14

#define I_IN_CHANNEL 0b00000000 //0
#define ZFB_IN_CHANNEL 0b00000001 //1

//SERIAL FIFO PINS
#define FIFO_OUT    SciaRegs.SCITXBUF.bit.TXDT //bit that transmits data
#define FIFO_IN_DATA_AVAILABLE  SciaRegs.SCIRXST.bit.RXRDY //goes high when a new char is ready to be read at serial in port otherwise goes low
#define FIFO_IN_DATA_AVAILABLE_SIZE SciaRegs.SCIFFRX.bit.RXFFST //indicates how many characters are ready to be read at serial in fifo
#define FIFO_IN SciaRegs.SCIRXBUF.bit.SAR //bit that receives data from in fifo


//#########################STM COMMAND PARAMETERS############################
#define DSP_DATA_SIZE 2048		//max number of data "slots" in dsp memory

#define DSPCMD_VERSION 'v'
#define IN_VERSION_SIZE		0
#define OUT_VERSION_SIZE	12//number of chars in reply: VVVVDDMMYYYY

#define DSPCMD_OUTPUT 'o'
#define	IN_OUTPUT_SIZE	2
#define	OUT_OUTPUT_SIZE	3			//number of additional chars to output (16 bits)

#define DSPCMD_INPUT 'i'
#define	IN_INPUT_CH_SIZE	1
#define	OUT_INPUT_CH_SIZE	2			//have to send one char (8 bits), but only 2 LS matter
//#define INPUT_DATA_SIZE 2		//read back two chars (16 bits) of data

#define DSPCMD_DELAY 'd'
#define IN_DELAY_SIZE	3
#define OUT_DELAY_SIZE	3			//max delay can be 0xFFFFFF cycles (about 0.2 sec)

#define DSPCMD_DELAYLONG 'D'
#define IN_DELAYLONG_SIZE	4
#define OUT_DELAYLONG_SIZE	3

#define DSPCMD_RAMP	'r'
#define IN_RAMP_SIZE	10
#define OUT_RAMP_SIZE	5

#define DSPCMD_RAMPREAD	'R'
#define	IN_RAMP_READ_SIZE 	11
#define	OUT_RAMP_READ_SIZE	3

#define DSPCMD_SQUAREWAVE 'q'
#define IN_SQUARE_SIZE	7           // 2 bytes each for high, low biases + 3 bytes for delay
#define OUT_SQUARE_SIZE	3

#define DSPCMD_TIPSETUP 't'
#define IN_TIPSETUP_SIZE	15      // 2 bytes each for xparab,xzero,zparab sizes (in cyles) +
#define OUT_TIPSETUP_SIZE	0	    // 3 bytes each for x,z multipliers + 3 bytes for delay factor

#define DSPCMD_GIANTSTEP 'g'
#define IN_GIANT_SIZE	2           // 2 bytes: 15 bits for num steps, 1 bit for direction (bit high = up, bit low = down)
#define OUT_GIANT_SIZE	3

#define DSPCMD_TRANSLATE_SAMPLE 'a'
#define IN_TRANSLATE_SAMPLE_SIZE 2
#define OUT_TRANSLATE_SAMPLE_SIZE 2   // 2 bytes: 15 bits for num steps, 1 bit for direction (bit high = up, bit low = down)

#define DSPCMD_TIPAPPROACH 'T'
#define IN_APPROACH_SIZE	7
#define OUT_APPROACH_SIZE	3		// 1 byte each for # giant & baby step size
// + 2 bytes for min tunneling current (in bits)
// + 3 bytes for baby parabola multiplier

#define DSPCMD_AUTOMINUS '-'
#define DSPCMD_AUTOPLUS '+'
#define IN_AUTO_SIZE	3
#define OUT_AUTO_SIZE	3				// 2 bytes for current z offset, 1 byte for ch

#define DSPCMD_STOP '!'

#define DSPCMD_SCANSETUP 's'
#define IN_SCANSETUP_SIZE	16
#define OUT_SCANSETUP_SIZE	0

#define DSPCMD_SCANLINE 'S'
#define IN_SCANLINE_SIZE	1
#define OUT_SCANLINE_SIZE	3

#define DSPCMD_HIGHRES 'H'
#define IN_HIGHRES_SIZE	    3
#define OUT_HIGHRES_SIZE	4

#define DSPCMD_HIGHERRES 'X'
#define IN_HIGHERRES_SIZE   3
#define OUT_HIGHERRES_SIZE  4

#define DSPCMD_TRACKSETUP 'k'
#define IN_TRACKSETUP_SIZE	13
#define OUT_TRACKSETUP_SIZE	1

#define DSPCMD_TRACK 'K'
#define IN_TRACK_SIZE	4
#define OUT_TRACK_SIZE	4		//Not sure about this one with the option to continue tracking after sending data.

#define DSPCMD_TRACKCONT 'c'

#define DSPCMD_RSSETUP 'Q'
#define IN_RSSETUP_SIZE	384		//3*N.  To match the old code, the maximum number of steps in a sequence is set to 64 steps, max 2 words per step.
#define OUT_RSSETUP_SIZE	3

#define DSPCMD_SPSETUP 'y'
#define IN_SPSETUP_SIZE		14
#define OUT_SPSETUP_SIZE	3

#define DSPCMD_SPEC 'Y'
#define IN_SPEC_SIZE	1
#define OUT_SPEC_SIZE	3		//Need to sort out how XDSO and ReadSeq will work in this system

#define DSPCMD_DIAGSETUP 'l'
#define IN_DIAGSETUP_SIZE	21
#define OUT_DIAGSETUP_SIZE	3

#define DSPCMD_DIAG 'L'
#define IN_DIAG_SIZE	2
#define OUT_DIAG_SIZE	3		//Variable, since it uses XDSO

#define DSPCMD_HOPSETUP 'j'
#define IN_HOPSETUP_SIZE	20
#define OUT_HOPSETUP_SIZE	3

#define DSPCMD_HOP 'J'
#define IN_HOP_SIZE		5	//These numbers are for each loop.  Same issues as ReadSeq
#define OUT_HOP_SIZE	6

#define DSPCMD_DEPSETUP 'p'
#define IN_DEPSETUP_SIZE	34
#define OUT_DEPSETUP_SIZE	3

#define DSPCMD_DEP 'P'
#define IN_DEP_SIZE		0
#define OUT_DEP_SIZE	3	//Uses XDSO equivalent

#define DSPCMD_DEP20SETUP 'u'
#define IN_DEP20SETUP_SIZE    36
#define OUT_DEP20SETUP_SIZE   3

#define DSPCMD_DEP20 'U'
#define IN_DEP20_SIZE     0
#define OUT_DEP20_SIZE    3   //Uses XDSO equivalent

#define DSPCMD_UPDATE '#'

#define DSPCMD_DAC 'a'
#define IN_DAC_SIZE		0
#define OUT_DAC_SIZE	32	//Two bytes for each output channel

#define DSPCMD_DEBUGGET 'b'
#define IN_DEBUGGET_SIZE	2
#define OUT_DEBUGGET_SIZE	3

#define DSPCMD_DEBUGSET 'B'
#define IN_DEBUGSET_SIZE	5	//1 for command, 2 for memory position, 3 for value.
#define OUT_DEBUGSET_SIZE	3

//########################### GLOBAL VARIABLES ##############################

// Analog Out.c
extern unsigned int lastOutput[NUM_OUT_CHANNEL];

// Analog In.c
extern char lastControlBits;
extern char ch13Bits;

// HighResOut.c
extern unsigned int savedHighResBits;

// AutoPlusMinus.c
extern unsigned int currentZOffset;

// TipApproach.c
extern unsigned long zOuterParabMult;
extern unsigned int zOuterParabSize;
extern unsigned long xOuterParabMult;
extern unsigned int xOuterParabSize;
extern unsigned int xOuterParabZeroCycles;
extern unsigned long tipApproachDelay;

extern unsigned int tipNumGiant;
extern unsigned int tipBabySize;
extern unsigned int tipMinI;
extern unsigned long tipBabyMult;

//BabySteps.c
extern unsigned int zOuterTunnel;

//Parabola.c
extern unsigned int stepNumber;

// Ramp.c
extern bool crashProtectionEnabled;
extern unsigned int outputChannel;
extern unsigned int inputChannel;
extern unsigned long currentOutputValue;

// ScanLine.c
extern unsigned int scanLimit;

extern bool flagZero;
extern unsigned int scanDeltaZ; //Used in Scan Protection
extern unsigned long scanReadDelay;

//##########################STRUCTURE DEFINITION#############################
#define MAXSIZE 2048

struct SerialData
{
    unsigned int numPoints;
    unsigned long data[MAXSIZE];
};

typedef struct SerialData data;

extern data readData;

//##########################FUNCTIONS PROTOTYPES#############################
void updateSerial(void);
void clearInFifo(void);
void nullDelay(void);
void delay(char * ,char *, unsigned long);
void dspInput(char * ,char *, char );
void dspOutput(char * ,char *, char , char , char );
void version(char * ,char * );
void ramp (char * ,char * );
void rampRead (char * ,char * );
void squareWave (char * ,char * );
void tipSetup (char * ,char * );
void giantStep (char * ,char * );
void oneGiant(unsigned int);
void tipApproach (char * , char * );
bool babyStep(unsigned int);
void autoPlus (char * ,char * );
void autoMinus (char * ,char * );
void highRes (char * ,char * );
void higherRes (char * ,char * );
void bitTest(void);
void scanLine(char * , char * );
void scanSetup(char * , char * );
void readSeqSetup(char * , char * , Uint16);
void specSetup(char * , char * );
void spec(char * , char *);
void autoSub(unsigned int);
void depSetup(char * , char *);
void deposition(char * , char *);
void dep20Setup(char *, char *);
void deposition20(char *, char *);
void trackSetup(char * , char *);
void track(char * , char *);
void giant(char * , char *);
void rampDiagSetup(char * , char *);
void rampDiag(char * , char *);
void delayLong(char * , char *);
void hopSetup(char * , char *);
void hop(char * , char *);
void dac(void);
void debugGet(char * , char *);
void debugSet(char * , char *);

unsigned int analogIn(unsigned int);
void analogOut(unsigned int, unsigned int);

Uint16 serialIn();

void serialOut_char(char);
void serialOut_two(unsigned int);
void serialOut_word(unsigned long);
void serialOut_xdso();
void serialOut_array(char *, unsigned int);

void highResOutput(unsigned int, unsigned int);
void higherResOutput(unsigned long);
void output20BitControlBits(char);

void readSeq();

unsigned int absin(unsigned int);

void pinInit();

void translateSample(char * ,char *);
void oneTranslate(unsigned int);

#endif /* DSP_FUNCTIONS_H_ */
