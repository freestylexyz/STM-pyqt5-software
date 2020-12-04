/*
 * PinInit.h
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_PININIT_H_
#define HEADERS_PININIT_H_

#include "F28x_Project.h"     // DSP28x Headerfile

//
// IN_DATA PINS
//
#define DACCLR   25
#define DACRST   27
#define CSB      26

#define ADCRST   122
#define CSA      61

#define CS20     95
#define RST20    139
#define CLR20    56

#define XGAIN_0  15
#define XGAIN_1  14
#define YGAIN_0  11
#define YGAIN_1  10
#define DITHER_0 9
#define COARSE   8
#define RETRACT  125
#define ROT      124
#define FEEDBACK 2
#define DITHER_1 3
#define ZGAIN_3  4
#define ZGAIN_0  5
#define ZGAIN_1  24
#define ZGAIN_2  16

//
// Function declaration
//
void outputPinInit(Uint16, Uint16);
void pinInit();

#endif /* HEADERS_PININIT_H_ */
//
// End of file
//
