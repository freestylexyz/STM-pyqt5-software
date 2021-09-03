/*
 * Aout.h
 *
 *  Created on: Nov 18, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_AOUT_H_
#define HEADERS_AOUT_H_

#include "DSP.h"

//
// 20bit DAC analog channel
//
#define DAC_20      0x20

//
// Function declaration
//
void aOut(char, Uint32);
void aOut_DSP();
void Square();
void sOut(char, Uint32, bool);

#endif /* HEADERS_AOUT_H_ */
