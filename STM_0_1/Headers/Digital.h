/*
 * Digital.h
 *
 *  Created on: Sep 28, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_DIGITAL_H_
#define HEADERS_DIGITAL_H_

#include "DSP.h"

//
// Command
//
# define Dither_0       0
# define Dither_1       1
# define Feedback       2
# define Retract        3
# define Coarse         4
# define Translation    5


//
// Function declaration
//
void gain_DSP();
void digitalO(Uint16, Uint16);
void digitalO_DSP();
void digitalScan_DSP();

#endif /* HEADERS_DIGITAL_H_ */
