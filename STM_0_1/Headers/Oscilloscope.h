/*
 * Oscilloscope.h
 *
 *  Created on: Nov 19, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_OSCILLOSCOPE_H_
#define HEADERS_OSCILLOSCOPE_H_

#include "DSP.h"

//
// Global variables declaration
//
extern Uint16 ldata[4096];

//
// Function declaration
//
void OSC_C_DSP();
void OSC_N_DSP();
void OSC_N(Uint16, Uint16, Uint16, Uint16);
void OSC_C(Uint16, Uint16, Uint16);
Uint16 OCS_U(Uint16, Uint16, Uint16, Uint16, Uint16, bool*);


#endif /* HEADERS_OSCILLOSCOPE_H_ */
