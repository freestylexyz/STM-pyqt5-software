/*
 * Ramp.h
 *
 *  Created on: Nov 18, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_RAMP_H_
#define HEADERS_RAMP_H_

#include "DSP.h"

//
// Function declaration
//
bool rampCore(bool, char, Uint32, Uint32, Uint32*, Uint32*, bool*);
void rampTo(char, Uint32, Uint32, Uint16, Uint16, bool);
void rampTo_S(char, Uint32, Uint32, Uint16);
void rampStep(char, Uint32, Uint32, Uint16, bool);
void rampTo_DSP();
void rampMeasure(char, Uint32, Uint32, Uint16, Uint16, bool);
void rampMeasure_DSP();


#endif /* HEADERS_RAMP_H_ */
