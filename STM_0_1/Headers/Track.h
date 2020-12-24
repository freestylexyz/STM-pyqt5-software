/*
 * Track.h
 *
 *  Created on: Dec 16, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_TRACK_H_
#define HEADERS_TRACK_H_

#include "DSP.h"

//
// Function declaration
//
void track();
void updatePos(bool, Uint16, Uint16, Uint32, Uint32, char, char, char*, char*,  Uint32*);

#endif /* HEADERS_TRACK_H_ */
