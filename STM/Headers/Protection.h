/*
 * Protection.h
 *
 *  Created on: Nov 19, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_PROTECTION_H_
#define HEADERS_PROTECTION_H_

#include "DSP.h"

//
// Global variables declaration
//
extern Uint16 stored_zofff;
extern bool stored_feedback;
extern bool tip_protect_status;

//
// Function declaration
//
bool protectRamp(bool, Uint16, bool*);
bool protectScan(char, Uint16, Uint32*, Uint32*, Uint32*);
void protectTip(bool, Uint16, bool, Uint16);
void protectTip_DSP();

#endif /* HEADERS_PROTECTION_H_ */
