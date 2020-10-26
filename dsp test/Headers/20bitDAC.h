/*
 * 20bitDAC.h
 *
 *  Created on: Sep 9, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_20BITDAC_H_
#define HEADERS_20BITDAC_H_

#include "F28x_Project.h"     // DSP28x Headerfile
#include "PinInit.h"
#include "SPI.h"
#include "Serial.h"

//
// Register address
//
#define BIT20_NOP           0x00
#define BIT20_DAC           0x10
#define BIT20_CONTROL       0x20
#define BIT20_CLEARCODE     0x30
#define BIT20_CONTROL_S     0x40

//
// Global variables declaration
//
extern Uint32 last20bit;

//
// Function declaration
//
void bit20_RST_H();
void bit20_CLR_H();
void bit20_W(char, Uint32);
void bit20_W_DSP();
void bit20_R_DSP();
void bit20Init();

#endif /* HEADERS_20BITDAC_H_ */
//
// End of file
//
