/*
 * DSP_FUNCTIONS.h
 *
 *  Created on: Jun 2, 2016
 *      Author: Ho Group
 *
 */

#ifndef HEADERS_DSP_H_
#define HEADERS_DSP_H_

#include <stdlib.h>

#include "F28x_Project.h"       // DSP28x header file
#include "Fundamental.h"        // Fundamental header file
#include "PinInit.h"            // Pin initial header file
#include "Serial.h"             // Serial communication header file
#include "SPI.h"                // SPI header file
#include "Status.h"             // Status header file
#include "20bitDAC.h"           // 20-bit DAC header file
#include "DAC.h"                // DAC header file
#include "ADC.h"                // ADC header file
#include "Digital.h"            // Digital header file
#include "Aout.h"               // Analog output header file
#include "Oscilloscope.h"       // Oscilloscope header file
#include "Auto.h"               // Auto function header file
#include "Protection.h"         // Protection header file
#include "Sequence.h"           // Sequence header file
#include "Ramp.h"               // Ramp header file
#include "TipApproach.h"        // Tip approach header file
#include "RampDiag.h"           // Ramp diagonal header file

//
// Function declaration
//
void commandLoop();
void testLoop();

//
// PC serial command
//
#define BIT20_W_DSP     0x20
#define BIT20_R_DSP     0x21

#define RAMPTO_DSP      0x31
#define RAMPMEASURE_DSP 0x32
#define RAMPDIAG_DSP    0x33

#define VERSION         0x70
#define LASTDIGITAL     0x72
#define LASTDAC         0x73
#define LAST20          0x74
#define OFFSET          0x75
#define DACRANGE        0x76
#define ADCRANGE        0x77
#define LOADOFFSET      0x78

#define GIANTSTEP       0x83
#define TIPAPPROACH     0x84

#define ADC_DSP         0xA0
#define ADC_N_DSP       0xA1
#define ADC_W_DSP       0xA2
#define ADC_R_DSP       0xA3

#define AOUT_DSP        0xA4
#define ZAUTO0_DAP      0xA6
#define SQUARE          0xA8
#define IAUTO_DSP       0xA9

#define OSCC            0xC0
#define OSCN            0xC1

#define DAC_W_DSP       0xD0
#define DAC_R_DSP       0xD1
#define DAC_RANGE       0xD2

#define DIGITALO_DSP    0xD3
#define GAIN_DSP        0xD4
#define DIGITALSCAN_DSP 0xD5

#define SERIALECHO      0xEC
#define FLOATT          0xFD

#endif /* HEADERS_DSP_H_ */
