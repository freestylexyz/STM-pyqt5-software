/*
 * Digital.h
 *
 *  Created on: Sep 28, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_DIGITAL_H_
#define HEADERS_DIGITAL_H_

#include "F28x_Project.h"     // DSP28x Headerfile
#include "PinInit.h"
#include "Serial.h"

//
// Command
//
# define dither_0       0
# define dither_1       1
# define feedback       2
# define retract        3
# define coarse         4
# define translation    5


//
// Global variables declaration
//
extern char lastdigital[14];

//
// Function declaration
//
void gain_DSP();
void digitalO(Uint16, Uint16);
void digitalO_DSP();
void digitalScan_DSP();

#endif /* HEADERS_DIGITAL_H_ */
