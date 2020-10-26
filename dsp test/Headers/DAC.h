/*
 * DAC.h
 *
 *  Created on: Sep 8, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_DAC_H_
#define HEADERS_DAC_H_

#include "F28x_Project.h"     // DSP28x Headerfile
#include "PinInit.h"
#include "SPI.h"
#include "Serial.h"

//
// Command Register
//
#define DAC_NOP             0x30
#define DAC_DEVICEID        0x01
#define DAC_STATUS          0x02
#define DAC_SPICONFIG       0x03
#define DAC_GENCONFIG       0x04
#define DAC_BRDCONFIG       0x05
#define DAC_SYNCCONFIG      0x06
#define DAC_TOGGLE0         0x07
#define DAC_TOGGLE1         0x08
#define DAC_PD              0x09
#define DAC_RANGE0          0x0A
#define DAC_RANGE1          0x0B
#define DAC_RANGE2          0x0C
#define DAC_RANGE3          0x0D
#define DAC_TRIGGER         0x0E
#define DAC_BRDCAST         0x0F
#define DAC_0               0x10
#define DAC_1               0x11
#define DAC_2               0x12
#define DAC_3               0x13
#define DAC_4               0x14
#define DAC_5               0x15
#define DAC_6               0x16
#define DAC_7               0x17
#define DAC_8               0x18
#define DAC_9               0x19
#define DAC_10              0x1A
#define DAC_11              0x1B
#define DAC_12              0x1C
#define DAC_13              0x1D
#define DAC_14              0x1E
#define DAC_15              0x1F
#define DAC_OFFSET0         0x20
#define DAC_OFFSET1         0x21
#define DAC_OFFSET2         0x22
#define DAC_OFFSET3         0x23

//
// Range command
//
#define DAC_U5       0b0000
#define DAC_U10      0b0001
#define DAC_U20      0b0010
#define DAC_U40      0b0100
#define DAC_B10      0b1001
#define DAC_B20      0b1010
#define DAC_B5       0b1110

//
// Global variables declaration
//
extern Uint16 dacrange[4];
extern Uint16 lastdac[16];

//
// Function declaration
//
void dac_RST_H();
void dac_CLR_H();
void dac_W(char, Uint16);
void dac_W_DSP();
void dac_R_DSP();
void dacInit();
void dacRange();

#endif /* HEADERS_DAC_H_ */

//
// End of file
//

