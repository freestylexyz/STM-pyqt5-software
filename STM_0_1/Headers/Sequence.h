/*
 * Sequence.h
 *
 *  Created on: Nov 18, 2020
 *      Author: yaoji
 */

#ifndef HEADERS_SEQUENCE_H_
#define HEADERS_SEQUENCE_H_

#include "DSP.h"

//
// Struct definination
//
struct sequence_command
{
    Uint16 seqnum;
    Uint16 commandlist[64];
    Uint32 datalist[64];
};

typedef struct sequence_command sc;

//
// Global variables declaration
//
extern sc ptSeq;

//
// Function declaration
//
void pointSeq(Uint32);
void setup_pointSeq();


#endif /* HEADERS_SEQUENCE_H_ */
