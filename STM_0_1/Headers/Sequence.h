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
    char commandlist[64];
    Uint32 datalist[64];
};

typedef struct sequence_command sc;

//
// Global variables declaration
//
extern sc ptSeq;
extern sc depSeqF;
extern sc depSeqB;

//
// Function declaration
//
void pointSeq(Uint32, sc);
void setup_pointSeq();
void setup_depSeq();


#endif /* HEADERS_SEQUENCE_H_ */
