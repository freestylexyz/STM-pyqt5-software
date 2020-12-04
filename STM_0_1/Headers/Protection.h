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
// Function declaration
//
bool protectRamp(bool, Uint16, bool*);
bool protectScan(char, Uint16, Uint32*, Uint32*, Uint32*, bool*);

#endif /* HEADERS_PROTECTION_H_ */
