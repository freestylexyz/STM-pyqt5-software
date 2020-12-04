/*
 * TipApproach.h
 *
 *  Created on: Nov 22, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_TIPAPPROACH_H_
#define HEADERS_TIPAPPROACH_H_

#include "DSP.h"

//
// Function declaration
//
void parabSlide(char, Uint16, Uint16);
void parabSteady(char, Uint16, Uint16);
bool babyStep(Uint16, Uint16, Uint16);
void oneGiant(char, Uint16, Uint16, Uint16, Uint16, Uint16);
void giantStep();
void tipApproach();

#endif /* HEADERS_TIPAPPROACH_H_ */
