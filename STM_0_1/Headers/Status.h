/*
 * Status.h
 *
 *  Created on: Nov 16, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_STATUS_H_
#define HEADERS_STATUS_H_

#include "DSP.h"

//
// Define
//
# define FIRMWARE_VERSION           "0.1 11162020"

//
// Global variables declaration
//
extern Uint16 dacrange[4];
extern Uint16 lastdac[16];
extern Uint16 offset[16];
extern char adcrange[8];
extern Uint32 last20bit;
extern char lastdigital[14];

//
// Function declaration
//
void version();
void status();
void lastdigital_O();
void initGlobal();
void loadOffset();

#endif /* HEADERS_STATUS_H_ */
